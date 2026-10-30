#include <algorithm>
#include <cmath>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <functional>
#include <string>
#include <vector>

#include "capture.hpp"
#include "model.hpp"
#include "store.hpp"

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

void test_store_roundtrip_and_corruption() {
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
    run("learns a pattern", test_learns_a_pattern);
    run("store roundtrip and corruption", test_store_roundtrip_and_corruption);
    run("fork diverges but remembers", test_fork_diverges_but_remembers);
    run("capture", test_capture);
    std::printf(failures ? "%d check(s) failed\n" : "all passed\n", failures);
    return failures ? 1 : 0;
}
