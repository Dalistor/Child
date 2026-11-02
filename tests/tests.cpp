#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iterator>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "capture/capture.hpp"
#include "model/model.hpp"
#include "engram/engram.hpp"
#include "neuron/neuron.hpp"
#include "synapse/synapse.hpp"
#include "dopamine/dopamine.hpp"

using namespace child;
namespace fs = std::filesystem;

namespace {

int failures = 0;

#define CHECK(cond)                                                       \
    do {                                                                  \
        if (!(cond)) {                                                    \
            std::printf("  FAIL %s:%d  %s\n", __FILE__, __LINE__, #cond); \
            ++failures;                                                   \
        }                                                                 \
    } while (0)

std::vector<uint8_t> bytes(const std::string& s) { return {s.begin(), s.end()}; }

void test_gradients_match_numeric() {
    Config cfg{4, 3, 8};
    Model m(cfg, 7);
    const uint8_t ctx[3] = {'a', 'b', 'c'};
    const uint8_t target = 'd';

    m.zero_grad();
    m.backward(ctx, target);
    std::vector<float> analytic = m.grads();

    auto loss_at = [&](size_t i, float delta) {
        float saved = m.params()[i];
        m.params()[i] = saved + delta;
        m.zero_grad();
        float l = m.backward(ctx, target);
        m.params()[i] = saved;
        return l;
    };

    // Embedding rows for the context bytes, plus a spread over the rest.
    std::vector<size_t> probe;
    for (uint8_t c : {uint8_t('a'), uint8_t('b'), uint8_t('c')})
        for (size_t e = 0; e < cfg.embed; ++e) probe.push_back(size_t(c) * cfg.embed + e);
    for (size_t i = kVocab * cfg.embed; i < m.params().size(); i += 37) probe.push_back(i);

    const float eps = 1e-2f;
    for (size_t i : probe) {
        float numeric = (loss_at(i, eps) - loss_at(i, -eps)) / (2 * eps);
        float tol = 1e-3f + 2e-2f * std::fabs(numeric);
        if (std::fabs(analytic[i] - numeric) > tol)
            std::printf("  param %zu: analytic %.6f numeric %.6f\n", i, analytic[i], numeric);
        CHECK(std::fabs(analytic[i] - numeric) <= tol);
    }
}

void test_virgin_network() {
    Config cfg;
    Model a(cfg, 1), b(cfg, 1), c(cfg, 2);
    CHECK(a.params() == b.params());
    CHECK(a.params() != c.params());

    // Untrained: close to uniform, loss near ln(256).
    std::vector<uint8_t> ctx(cfg.context, 0);
    float probs[kVocab];
    a.predict(ctx.data(), probs);
    float sum = 0;
    for (float p : probs) sum += p;
    CHECK(std::fabs(sum - 1.f) < 1e-4f);
    CHECK(*std::max_element(probs, probs + kVocab) < 0.05f);
}

// Pins the exact weights a known seed and text produce, so a change in
// initialization, learning or compiler floating point shows up on any machine.
void test_reference_weights() {
    Model m(Config{}, 42);
    learn(m, bytes("int main() { return 0; }\n"), LearnOptions{});

    uint64_t h = 0xcbf29ce484222325ull;  // FNV-1a 64
    const auto* p = reinterpret_cast<const unsigned char*>(m.params().data());
    for (size_t i = 0; i < m.params().size() * sizeof(float); ++i) h = (h ^ p[i]) * 0x100000001b3ull;

    const uint64_t expected = 0x6213932d187d6039ull;
    if (h != expected) std::printf("  weights hash 0x%016llx\n", static_cast<unsigned long long>(h));
    CHECK(h == expected);
}

// NEU-01: an AdEx neuron rests without input, fires repeatedly under a
// constant current with growing intervals, resets after each spike and is
// fully deterministic.
std::vector<float> spike_times(Neuron& n, float current, float duration_ms) {
    std::vector<float> times;
    const int steps = int(duration_ms / kStepMs);
    for (int i = 1; i <= steps; ++i)
        if (n.step(current)) times.push_back(float(i) * kStepMs);
    return times;
}

void test_adex_neuron() {
    Neuron idle;
    CHECK(spike_times(idle, 0.f, 1000.f).empty());
    CHECK(std::fabs(idle.potential() - idle.params().rest) < 1e-3f);

    Neuron driven;
    std::vector<float> spikes = spike_times(driven, 1000.f, 500.f);
    std::printf("  %zu spikes, first intervals", spikes.size());
    for (size_t i = 1; i < spikes.size() && i < 5; ++i) std::printf(" %.1f", spikes[i] - spikes[i - 1]);
    std::printf(" ms\n");
    CHECK(spikes.size() >= 4);
    for (size_t i = 2; i < spikes.size() && i < 4; ++i)
        CHECK(spikes[i] - spikes[i - 1] > spikes[i - 1] - spikes[i - 2]);

    // Each spike resets the potential and raises adaptation by about b.
    Neuron n;
    int resets = 0;
    for (int i = 0; i < 5000 && resets < 3; ++i) {
        float w_before = n.adaptation();
        if (!n.step(1000.f)) continue;
        ++resets;
        CHECK(n.potential() == n.params().reset);
        CHECK(n.adaptation() - w_before > 0.9f * n.params().spike_jump);
    }
    CHECK(resets == 3);

    Neuron again;
    CHECK(spike_times(again, 1000.f, 500.f) == spikes);
}

// SIN-01: the weight change depends on the order and the interval between
// pre- and postsynaptic spikes, stays within [0, max] and needs spikes.
float pair_change(float pre, float post) {
    Synapse s(0.5f);
    for (auto [t, is_pre] : {std::pair{std::min(pre, post), pre <= post}, std::pair{std::max(pre, post), pre > post}})
        s.apply(is_pre ? s.on_pre(t) : s.on_post(t));
    return s.weight() - 0.5f;
}

void test_stdp_synapse() {
    const float near = pair_change(0.f, 5.f), mid = pair_change(0.f, 20.f), far = pair_change(0.f, 60.f);
    const float rev_near = pair_change(5.f, 0.f), rev_far = pair_change(60.f, 0.f);
    std::printf("  pre->post +5 %.5f  +20 %.5f  +60 %.5f | post->pre 5 %.5f  60 %.5f\n", near, mid, far,
                rev_near, rev_far);
    CHECK(near > 0.f);
    CHECK(rev_near < 0.f);
    CHECK(near > mid && mid > far && far > 0.f);
    CHECK(rev_near < rev_far && rev_far < 0.f);

    Synapse up(0.5f), down(0.5f);
    for (int i = 0; i < 2000; ++i) {
        const float t = 100.f * float(i);
        up.apply(up.on_pre(t));
        up.apply(up.on_post(t + 5.f));
        down.apply(down.on_post(t));
        down.apply(down.on_pre(t + 5.f));
    }
    CHECK(up.weight() == up.params().max_weight);
    CHECK(down.weight() == 0.f);

    Synapse quiet(0.5f);
    CHECK(quiet.weight() == 0.5f);
    for (int i = 0; i < 100; ++i) quiet.apply(quiet.on_pre(10.f * float(i)));  // pre alone never pairs
    CHECK(quiet.weight() == 0.5f);
}

// DOP-01: a spike pair only changes the weight if dopamine arrives while its
// eligibility trace is still alive. `release_at` < 0 means no dopamine.
float three_factor_change(bool pre_first, float release_at) {
    Synapse s(0.5f);
    Dopamine dopamine;
    s.tag(pre_first ? s.on_pre(0.f) : s.on_post(0.f), 0.f);
    s.tag(pre_first ? s.on_post(5.f) : s.on_pre(5.f), 5.f);

    const float dt = 1.f;
    for (int i = 6; i <= 10000; ++i) {
        const float t = float(i) * dt;
        if (release_at >= 0.f && t == release_at) dopamine.release(1.f);
        dopamine.step(dt);
        s.reinforce(dopamine.level(), t, dt);
    }
    return s.weight() - 0.5f;
}

void test_three_factor_plasticity() {
    const float soon = three_factor_change(true, 200.f), later = three_factor_change(true, 1000.f);
    const float never = three_factor_change(true, -1.f), expired = three_factor_change(true, 6000.f);
    const float reversed = three_factor_change(false, 200.f);
    std::printf("  dopamine at 0.2 s %+.5f  1 s %+.5f  6 s %+.5f  none %+.5f | reversed pair %+.5f\n", soon,
                later, expired, never, reversed);
    CHECK(soon > 0.f);
    CHECK(later > 0.f && later < soon);
    CHECK(reversed < 0.f);
    CHECK(never == 0.f);
    CHECK(expired == 0.f);

    Synapse s(0.5f);
    s.tag(0.01f, 0.f);
    CHECK(s.weight() == 0.5f);  // tagging alone never touches the weight
    CHECK(s.eligibility(4999.f) > 0.f);
    CHECK(s.eligibility(5000.f) == 0.f);
}

void test_learns_a_pattern() {
    Model m(Config{16, 8, 64}, 3);
    std::vector<uint8_t> data = bytes("int main() { return 0; }\n");

    LearnOptions opts;
    opts.epochs = 400;
    opts.lr = 0.2f;
    opts.batch = 8;
    float first = 0, last = 0;
    last = learn(m, data, opts, [&](int epoch, float loss) {
        if (epoch == 1) first = loss;
    });
    std::printf("  loss %.3f -> %.3f\n", first, last);
    CHECK(first > 4.f);
    CHECK(last < 0.1f);

    // It now predicts the next byte of what it learned.
    size_t right = 0;
    std::vector<uint8_t> ctx(8);
    float probs[kVocab];
    for (size_t i = 0; i < data.size(); ++i) {
        window(data, i, 8, ctx.data());
        m.predict(ctx.data(), probs);
        right += size_t(std::max_element(probs, probs + kVocab) - probs) == data[i];
    }
    CHECK(right == data.size());
}

std::string load_error(const std::string& path) {
    try {
        load(path);
    } catch (const std::exception& e) {
        return e.what();
    }
    return "";
}

// Saves `s`, then patches the format version (when `version` != 0) or the
// first section's type, and re-seals the checksum so only that change counts.
std::string rewritten(const std::string& path, const Snapshot& s, std::optional<uint32_t> type,
                      uint32_t version) {
    save(path, s);
    std::ifstream in(path, std::ios::binary);
    std::vector<uint8_t> buf{std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>()};
    in.close();

    const size_t header = 4 + 4 + 8 * 3 + 4 + s.lineage.name.size() + 4;  // up to section count
    if (version) std::memcpy(buf.data() + 4, &version, sizeof version);
    if (type) std::memcpy(buf.data() + header, &*type, sizeof *type);

    uint64_t h = 0xcbf29ce484222325ull;
    for (size_t i = 0; i + 8 < buf.size(); ++i) h = (h ^ buf[i]) * 0x100000001b3ull;
    std::memcpy(buf.data() + buf.size() - 8, &h, sizeof h);

    std::string out = path + ".patched";
    std::ofstream(out, std::ios::binary).write(reinterpret_cast<const char*>(buf.data()), std::streamsize(buf.size()));
    return out;
}

void test_engram_roundtrip_and_corruption() {
    fs::path dir = "child_test_tmp";
    fs::create_directories(dir);
    std::string path = (dir / "a.child").string();

    Snapshot s = create(Config{8, 4, 16}, 5, "alpha");
    s.lineage.steps = 123;
    save(path, s);

    Snapshot back = load(path);
    CHECK(back.config == s.config);
    CHECK(back.lineage.id == s.lineage.id);
    CHECK(back.lineage.parent == 0);
    CHECK(back.lineage.steps == 123);
    CHECK(back.lineage.name == "alpha");
    CHECK(back.params == s.params);
    CHECK(!fs::exists(path + ".tmp"));

    // Flip one byte in the middle: the checksum must catch it.
    {
        std::fstream f(path, std::ios::binary | std::ios::in | std::ios::out);
        f.seekp(100);
        char c;
        f.seekg(100);
        f.get(c);
        f.seekp(100);
        f.put(char(c ^ 0x5a));
    }
    bool threw = false;
    try {
        load(path);
    } catch (const std::exception&) {
        threw = true;
    }
    CHECK(threw);

    threw = false;
    try {
        load((dir / "missing.child").string());
    } catch (const std::exception&) {
        threw = true;
    }
    CHECK(threw);

    // A well-formed engram whose section is unknown, or a format-1 file, must
    // be refused with a message that says why, never read half-way.
    CHECK(load_error(rewritten(path, s, 99, 0)).find("section type 99") != std::string::npos);
    CHECK(load_error(rewritten(path, s, std::nullopt, 1)).find("recreate") != std::string::npos);

    fs::remove_all(dir);
}

void test_fork_diverges_but_remembers() {
    Snapshot root = create(Config{8, 4, 16}, 9, "root");
    Snapshot a = fork(root, "a");
    Snapshot b = fork(root, "b");

    CHECK(a.lineage.parent == root.lineage.id);
    CHECK(b.lineage.parent == root.lineage.id);
    CHECK(a.lineage.id != b.lineage.id);
    CHECK(a.lineage.id != root.lineage.id);
    CHECK(a.params == root.params);

    // Teach only branch a; root and b must stay untouched.
    Model ma(a.config, a.params);
    LearnOptions opts;
    opts.epochs = 3;
    learn(ma, bytes("hello"), opts);
    a.params = ma.params();

    CHECK(a.params != root.params);
    CHECK(b.params == root.params);
}

void test_capture() {
    fs::path dir = "child_test_tmp";
    fs::remove_all(dir);
    fs::create_directories(dir / "src");
    fs::create_directories(dir / ".git");
    fs::create_directories(dir / "build");

    auto write = [](const fs::path& p, const std::string& body) { std::ofstream(p, std::ios::binary) << body; };
    write(dir / "src" / "b.c", "bb");
    write(dir / "src" / "a.c", "aa");
    write(dir / ".git" / "x", "hidden");
    write(dir / "build" / "y", "output");
    write(dir / "blob.bin", std::string("ab\0cd", 5));

    std::vector<uint8_t> got = capture(dir.string());
    CHECK(got == (std::vector<uint8_t>{'a', 'a', 0, 'b', 'b', 0}));

    bool threw = false;
    try {
        capture((dir / "nope").string());
    } catch (const std::exception&) {
        threw = true;
    }
    CHECK(threw);

    fs::remove_all(dir);
}

void run(const char* name, const std::function<void()>& fn) {
    int before = failures;
    std::printf("%s\n", name);
    fn();
    std::printf("  %s\n", failures == before ? "ok" : "FAILED");
}

}  // namespace

int main() {
    run("gradients match numeric", test_gradients_match_numeric);
    run("virgin network", test_virgin_network);
    run("reference weights", test_reference_weights);
    run("learns a pattern", test_learns_a_pattern);
    run("adex neuron", test_adex_neuron);
    run("stdp synapse", test_stdp_synapse);
    run("three-factor plasticity", test_three_factor_plasticity);
    run("engram roundtrip and corruption", test_engram_roundtrip_and_corruption);
    run("fork diverges but remembers", test_fork_diverges_but_remembers);
    run("capture", test_capture);
    std::printf(failures ? "%d check(s) failed\n" : "all passed\n", failures);
    return failures ? 1 : 0;
}
