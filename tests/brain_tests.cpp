// Behaviour of the integrated brain: thalamus, cortex with dendrites,
// cerebellum, dopamine as surprise, hippocampus and sleep, neurogenesis,
// parameters and the dynamic state in the engram.
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <stdexcept>
#include <string>
#include <vector>

#include "brain/brain.hpp"
#include "brain/parameters.hpp"
#include "engram/engram.hpp"
#include "harness.hpp"
#include "model/model.hpp"
#include "neuron/dendrite.hpp"
#include "neuron/neuron.hpp"

using namespace child;

namespace {

std::string repeat(const std::string& s, int n) {
    std::string out;
    for (int i = 0; i < n; ++i) out += s;
    return out;
}

// A smaller brain than the default, so the suite stays fast.
BrainParams small() {
    BrainParams p;
    configure(p, "cortex.excitatory = 128\ncortex.interneurons = 32\nthalamus.channels = 128\n");
    return p;
}

double train(Brain& brain, const std::string& text, int epochs) {
    double mean = 0;
    for (int e = 0; e < epochs; ++e) {
        double sum = 0;
        for (unsigned char c : text) sum += brain.perceive(c, true);
        mean = sum / double(text.size());
    }
    return mean;
}

struct Reading {
    double surprise = 0, accuracy = 0, dopamine = 0;
    std::vector<float> each;  // surprise of every byte
};

// Perceives `text` without learning, on a copy, so the brain itself is untouched.
Reading read(Brain brain, const std::string& text) {
    Reading r;
    for (unsigned char c : text) {
        const float* p = brain.prediction();
        r.accuracy += std::max_element(p, p + kOutputs) - p == c;
        r.each.push_back(brain.perceive(c, false));
        r.surprise += r.each.back();
    }
    r.surprise /= double(text.size());
    r.accuracy /= double(text.size());
    return r;
}

double pearson(const std::vector<float>& x, const std::vector<float>& y) {
    double mx = 0, my = 0;
    for (size_t i = 0; i < x.size(); ++i) mx += x[i], my += y[i];
    mx /= double(x.size()), my /= double(y.size());
    double sxy = 0, sxx = 0, syy = 0;
    for (size_t i = 0; i < x.size(); ++i) {
        sxy += (x[i] - mx) * (y[i] - my);
        sxx += (x[i] - mx) * (x[i] - mx);
        syy += (y[i] - my) * (y[i] - my);
    }
    return sxy / std::sqrt(sxx * syy);
}

const std::string kSentence = "the quick brown fox jumps over the lazy dog. ";

// TAL-01: each byte is a fixed sparse set of relay channels that fire while it is presented.
void test_thalamus() {
    ThalamusParams p;
    Thalamus a(p, 3), b(p, 3);
    int distinct = 0;
    for (int x = 0; x < 256; ++x) {
        bool unique = true;
        for (int y = 0; y < x && unique; ++y) unique = a.code(uint8_t(x)) != a.code(uint8_t(y));
        distinct += unique;
    }
    std::vector<uint32_t> fired, again;
    a.relay('a');
    b.relay('a');
    for (int i = 0; i < 1000; ++i) a.step(0.1f, fired), b.step(0.1f, again);  // 100 ms
    const auto& code = a.code('a');
    size_t on = 0;
    for (uint32_t c : fired) on += std::binary_search(code.begin(), code.end(), c);
    const double on_rate = double(on) / double(code.size()) / 0.1;
    const double off_rate = double(fired.size() - on) / double(p.channels - code.size()) / 0.1;
    std::printf("  %d distinct codes of %u channels; 'a' channels %.0f Hz, others %.1f Hz\n", distinct,
                p.channels_per_byte, on_rate, off_rate);
    CHECK(distinct == 256);
    CHECK(on_rate > 100.0 && off_rate < 10.0);
    CHECK(fired == again);
}

// CER-01: the cerebellar readout predicts the next byte well above chance on a repetitive source.
void test_cerebellar_readout() {
    const std::string text = repeat(kSentence, 20);
    Brain brain(small(), 1);
    const Reading before = read(brain, text);
    train(brain, text, 3);
    const Reading after = read(brain, text);
    std::printf("  %zu bytes: accuracy %.3f -> %.3f, surprise %.2f -> %.2f nats (chance: 1/256, ln 256 = 5.55)\n",
                text.size(), before.accuracy, after.accuracy, before.surprise, after.surprise);
    CHECK(after.accuracy >= 0.5);
    CHECK(after.surprise <= std::log(256.0) / 2);
}

// SUR-01: dopamine is the unexpected part of the surprise, and the surprise
// follows the MLP oracle: the same bytes are hard for both.
void test_surprise_as_dopamine() {
    const std::string text = repeat(kSentence, 20), novel = repeat("pack my box with five dozen liquor jugs! ", 4);
    Brain brain(small(), 2);
    double first = 0, last = 0, dopamine_first = 0, dopamine_last = 0;
    for (int e = 0; e < 3; ++e) {
        double sum = 0, released = 0;
        for (unsigned char c : text) sum += brain.perceive(c, true), released += brain.dopamine();
        (e == 0 ? first : last) = sum / double(text.size());
        (e == 0 ? dopamine_first : dopamine_last) = released / double(text.size());
    }
    // Dopamine for the learned text and for a novel one, from the same trained brain.
    auto released_on = [&](const std::string& s) {
        Brain copy = brain;
        double released = 0;
        for (unsigned char c : s) copy.perceive(c, true), released += copy.dopamine();
        return released / double(s.size());
    };
    const double on_learned = released_on(text.substr(0, novel.size())), on_novel = released_on(novel);

    Model mlp(Config{}, 2);
    LearnOptions opts;
    opts.epochs = 3;
    const std::vector<uint8_t> data(text.begin(), text.end());
    const float mlp_loss = learn(mlp, data, opts);
    std::vector<float> mlp_each, probs(kVocab);
    std::vector<uint8_t> ctx(mlp.config().context);
    for (size_t i = 0; i < data.size(); ++i) {
        window(data, i, mlp.config().context, ctx.data());
        mlp.predict(ctx.data(), probs.data());
        mlp_each.push_back(-std::log(std::max(probs[data[i]], 1e-30f)));
    }
    const double r = pearson(read(brain, text).each, mlp_each);
    std::printf("  surprise %.2f -> %.2f nats over 3 epochs (mlp loss %.2f); dopamine per byte %.3f -> %.3f\n", first,
                last, mlp_loss, dopamine_first, dopamine_last);
    std::printf("  dopamine on the learned text %.3f, on a novel one %.3f; per-byte correlation with the mlp %.2f\n",
                on_learned, on_novel, r);
    // The expected surprise falls with learning, so dopamine per byte need not
    // fall; what must hold is that the unexpected releases more.
    CHECK(last < 0.5 * first);
    CHECK(on_novel > 2 * on_learned);
    CHECK(r >= 0.3);
}

// DEN-01: a calcium dendrite (Gidon et al. 2020) lets one neuron compute XOR;
// the same neuron without it, driven in proportion to its input, cannot.
int spikes_for(float drive, bool calcium) {
    CalciumParams ca;
    ca.peak_current = 800.f;
    const float current = calcium ? ca.peak_current * calcium_amplitude(drive, ca) : 400.f * drive;
    Neuron n;
    int spikes = 0;
    for (int i = 0; i < 1000; ++i) spikes += n.step(current);  // 100 ms
    return spikes;
}

void test_dendritic_xor() {
    int xor_ok = 0, point_ok = 0;
    std::printf("  x y | calcium dendrite | point neuron\n");
    for (int x = 0; x <= 1; ++x)
        for (int y = 0; y <= 1; ++y) {
            const int with = spikes_for(float(x + y), true), without = spikes_for(float(x + y), false);
            std::printf("  %d %d | %2d spikes        | %2d spikes\n", x, y, with, without);
            xor_ok += (with > 0) == (x != y);
            point_ok += (without > 0) == (x != y);
        }
    CHECK(xor_ok == 4);
    CHECK(point_ok < 4);  // monotonic in its input, so either x+y=1 and x+y=2 both fire or neither
}

// DEN-02: after "abcabd" the brain tells apart the two contexts of "ab": it
// predicts c after "dab" and d after "cab".
void test_sequence_contexts() {
    const std::string text = repeat("abcabd", 200);
    for (uint32_t segments : {2u, 0u}) {
        BrainParams p = small();
        p.cortex.segments = segments;
        Brain brain(p, 3);
        train(brain, text, 3);
        Brain probe = brain;
        int right_c = 0, right_d = 0, n_c = 0, n_d = 0;
        for (size_t i = 0; i < text.size(); ++i) {
            const float* pr = probe.prediction();
            const auto guess = char(std::max_element(pr, pr + kOutputs) - pr);
            if (i >= 3 && text[i - 1] == 'b') {
                (text[i] == 'c' ? n_c : n_d) += 1;
                (text[i] == 'c' ? right_c : right_d) += guess == text[i];
            }
            probe.perceive(uint8_t(text[i]), false);
        }
        std::printf("  %u segments per cell: after 'ab', c predicted %.2f, d predicted %.2f (predicted cells %u)\n",
                    segments, double(right_c) / n_c, double(right_d) / n_d, brain.cortex().predicted());
        if (segments) {
            CHECK(double(right_c) / n_c >= 0.9);
            CHECK(double(right_d) / n_d >= 0.9);
        }
    }
}

// HIP-01: learning B after A spoils A unless the brain sleeps, replaying the
// hippocampal episodes interleaved.
void test_sleep_protects() {
    const std::string a = repeat("the cat sat on the mat. ", 25), b = repeat("a dog ran in the fog. ", 25);
    Brain base(small(), 4);
    train(base, a, 3);
    const double learned = read(base, a).surprise;
    Brain awake = base, sleeper = base;
    for (int e = 0; e < 3; ++e) {
        train(awake, b, 1);
        train(sleeper, b, 1);
        sleeper.sleep(2);
    }
    const Reading awake_a = read(awake, a), sleeper_a = read(sleeper, a);
    const Reading awake_b = read(awake, b), sleeper_b = read(sleeper, b);
    std::printf("  A after learning A: %.2f nats\n", learned);
    std::printf("  after B without sleep: A %.2f, B %.2f | with sleep: A %.2f, B %.2f\n", awake_a.surprise,
                awake_b.surprise, sleeper_a.surprise, sleeper_b.surprise);
    CHECK(awake_a.surprise - learned > 0.3);
    CHECK(sleeper_a.surprise - learned <= 0.5 * (awake_a.surprise - learned));
    CHECK(sleeper_b.surprise <= std::log(256.0) / 2);
}

// HIP-02: a stored episode is recalled from half of its pattern (CA3 completion).
void test_pattern_completion() {
    HippocampusParams p;
    p.capacity = 200;
    Hippocampus hippocampus(p);
    Rng rng(5);
    std::vector<std::vector<uint32_t>> patterns;
    for (int e = 0; e < 200; ++e) {
        std::vector<uint32_t> cells;
        while (cells.size() < 40) {
            const auto c = uint32_t(rng.below(8192));
            if (std::find(cells.begin(), cells.end(), c) == cells.end()) cells.push_back(c);
        }
        std::sort(cells.begin(), cells.end());
        patterns.push_back(cells);
        hippocampus.store(cells, uint8_t(e), rng);
    }
    int recalled = 0;
    for (int e = 0; e < 200; ++e) {
        std::vector<uint32_t> cue;
        for (size_t i = 0; i < patterns[size_t(e)].size(); i += 2) cue.push_back(patterns[size_t(e)][i]);
        const int64_t got = hippocampus.recall(cue);
        recalled += got >= 0 && hippocampus.next(uint32_t(got)) == uint8_t(e);
    }
    std::printf("  %d of 200 episodes recalled from half of their cells\n", recalled);
    CHECK(recalled == 200);
}

// NEO-01: neurons are born when the surprise stays high and stops falling,
// never past the ceiling, and not when the brain copes.
BrainParams growing() {
    BrainParams p = small();
    configure(p, "neurogenesis.window = 256\nneurogenesis.batch = 16\nneurogenesis.max_neurons = 176\n");
    return p;
}

std::string noise(size_t n, uint64_t seed) {
    Rng rng(seed);
    std::string s;
    for (size_t i = 0; i < n; ++i) s += char(' ' + rng.below(95));
    return s;
}

void test_neurogenesis() {
    const std::string hard = noise(1500, 9), easy = repeat("abab", 375);
    Brain grows(growing(), 6), copes(growing(), 6), off(growing(), 6);
    off.neurogenesis().enabled = 0;
    train(grows, hard, 1);
    train(copes, easy, 1);
    train(off, hard, 1);
    // The newest cells must take part: they fire while the hard text goes on.
    Brain probe = grows;
    long young_spikes = 0;
    for (unsigned char c : hard.substr(0, 300)) {
        probe.perceive(c, true);
        const auto& counts = probe.cortex().counts();
        for (size_t i = 128; i < counts.size(); ++i) young_spikes += counts[i];
    }
    std::printf("  hard text: %u cells (%llu born); easy text: %u; neurogenesis off: %u; spikes of born cells %ld\n",
                grows.cortex().size(), (unsigned long long)grows.born(), copes.cortex().size(), off.cortex().size(),
                young_spikes);
    CHECK(grows.cortex().size() == 176);
    CHECK(grows.cerebellum().mossy() == growing().thalamus.channels + 176);
    CHECK(copes.cortex().size() == 128);
    CHECK(off.cortex().size() == 128);
    CHECK(young_spikes > 0);

    // Growth is deterministic and survives the engram.
    Brain twin(growing(), 6);
    train(twin, hard, 1);
    const Brain restored = Brain::deserialize(grows.serialize());
    CHECK(twin.serialize() == grows.serialize());
    CHECK(restored.cortex().size() == 176 && restored.serialize() == grows.serialize());
}

// ENG-02: the engram keeps the dynamic state: continuing from a saved brain
// gives the same spikes as continuing without saving; parent and new fork too.
void test_dynamic_state() {
    const std::string text = repeat(kSentence, 6);
    Brain brain(small(), 7);
    train(brain, text.substr(0, 150), 1);

    Snapshot parent;
    parent.lineage = {new_id(), 0, 150, "parent"};
    parent.brain = brain.serialize();
    const std::string path = "child_dynamic_state.child";
    save(path, parent);
    Brain resumed = Brain::deserialize(load(path).brain);
    Brain branch = Brain::deserialize(fork(load(path), "branch").brain);
    std::remove(path.c_str());

    size_t same = 0, same_branch = 0, steps = 0;
    for (unsigned char c : text.substr(150)) {
        brain.perceive(c, true);
        resumed.perceive(c, true);
        branch.perceive(c, true);
        same += brain.cortex().counts() == resumed.cortex().counts();
        same_branch += brain.cortex().counts() == branch.cortex().counts();
        ++steps;
    }
    std::printf("  %zu bytes after saving: identical spikes in %zu (resumed) and %zu (fork)\n", steps, same,
                same_branch);
    CHECK(same == steps && same_branch == steps);
    CHECK(brain.serialize() == resumed.serialize());
}

// CFG-01 / PAR-01: parameters come from text, are checked against their
// registered range, and every default lies inside it.
void test_parameters() {
    BrainParams p;
    int outside = 0;
    for (const auto& q : parameters(p)) outside += !q.admits(q.get());
    CHECK(outside == 0);

    auto error = [](const std::string& text) {
        BrainParams q;
        try {
            configure(q, text);
        } catch (const std::exception& e) {
            return std::string(e.what());
        }
        return std::string();
    };
    const std::string range = error("neuron.capacitance = 1000"), unknown = error("neuron.colour = 3");
    const std::string malformed = error("synapse.tau_plus = fast"), fraction = error("cortex.excitatory = 10.5");
    std::printf("  %s | %s | %s\n", range.c_str(), unknown.c_str(), malformed.c_str());
    CHECK(range.find("neuron.capacitance") != std::string::npos);
    CHECK(unknown.find("neuron.colour") != std::string::npos);
    CHECK(malformed.find("synapse.tau_plus") != std::string::npos);
    CHECK(fraction.find("cortex.excitatory") != std::string::npos);

    BrainParams q;
    configure(q, "# a comment\nneuron.capacitance = 200  # pF\n");
    CHECK(q.cortex.neuron.capacitance == 200.f && q.cortex.neuron.leak == AdExParams{}.leak);
    BrainParams round;
    configure(round, describe(q));
    CHECK(describe(round) == describe(q));
    CHECK(Brain(q, 1).serialize() == Brain(round, 1).serialize());
    CHECK(Brain(q, 1).serialize() != Brain(BrainParams{}, 1).serialize());
}

}  // namespace

void run_brain_tests() {
    run("thalamus", test_thalamus);
    run("cerebellar readout", test_cerebellar_readout);
    run("surprise as dopamine", test_surprise_as_dopamine);
    run("dendritic xor", test_dendritic_xor);
    run("sequence contexts", test_sequence_contexts);
    run("sleep protects", test_sleep_protects);
    run("pattern completion", test_pattern_completion);
    run("neurogenesis", test_neurogenesis);
    run("dynamic state", test_dynamic_state);
    run("parameters", test_parameters);
}
