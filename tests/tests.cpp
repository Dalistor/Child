#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iterator>
#include <limits>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "capture/capture.hpp"
#include "model/model.hpp"
#include "engram/engram.hpp"
#include "neuron/neuron.hpp"
#include "synapse/synapse.hpp"
#include "synapse/homeostasis.hpp"
#include "dopamine/dopamine.hpp"
#include "interneuron/interneuron.hpp"
#include "thalamus/thalamus.hpp"
#include "model/rng.hpp"

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

// Lowest and highest potential over `duration_ms` without input current.
std::pair<float, float> potential_range(Neuron& n, float duration_ms) {
    float low = n.potential(), high = n.potential();
    for (int i = 0; i < int(duration_ms / kStepMs); ++i) {
        n.step(0.f);
        low = std::min(low, n.potential());
        high = std::max(high, n.potential());
    }
    return {low, high};
}

void test_synaptic_transmission() {
    const float rest = AdExParams{}.rest;

    Neuron excited;
    excited.excite(5.f);
    // Subthreshold adaptation (a > 0) makes a small rebound past rest afterwards.
    const float e_high = potential_range(excited, 200.f).second;
    Neuron inhibited;
    inhibited.inhibit(5.f);
    const float i_low = potential_range(inhibited, 200.f).first;
    std::printf("  5 nS excitatory peak %+.3f mV, inhibitory trough %+.3f mV (from rest)\n", e_high - rest,
                i_low - rest);
    CHECK(e_high > rest + 0.5f);
    CHECK(std::fabs(excited.potential() - rest) < 0.05f);
    CHECK(excited.excitation() < 1e-6f);
    CHECK(i_low < rest - 0.5f);
    CHECK(std::fabs(inhibited.potential() - rest) < 0.05f);

    // However strong, inhibition pulls the potential toward E_I and never past it.
    Neuron clamped;
    clamped.inhibit(1000.f);
    CHECK(potential_range(clamped, 100.f).first >= ReceptorParams{}.inhibitory_reversal);

    Neuron idle;
    CHECK(potential_range(idle, 200.f).second < rest + 1e-3f);

    SpikeSource a(9, 20.f), b(9, 20.f);
    int spikes = 0, same = 0;
    for (int i = 0; i < 100000; ++i) {
        const int fired = a.step(kStepMs);
        spikes += fired;
        same += fired == b.step(kStepMs);
    }
    std::printf("  20 Hz source: %d spikes in 10 s\n", spikes);
    CHECK(same == 100000);
    CHECK(spikes > 150 && spikes < 250);
}

// HOM-01: one neuron driven by background excitatory inputs. Homeostasis
// brings it to the target rate; then the input rate doubles and the rate is
// measured in 20 s windows, with the scaling kept on or switched off.
constexpr float kMaxConductance = 1.f;  // nS at full weight (Song et al. 2000 range)

struct ScalingRun {
    float before;  // Hz, last 20 s before the input doubles
    float after;   // Hz, 20 s window ending 50 s after it doubles
    std::vector<float> rates;  // Hz per 10 s, after the doubling
    float ratio_before, ratio_after;  // between two effective incoming weights
};

ScalingRun run_scaling(bool keep_homeostasis) {
    constexpr int kInputs = 200;
    constexpr float kRate = 20.f;  // Hz per input (Song et al. 2000 use 10-40)
    std::vector<SpikeSource> sources;
    std::vector<Synapse> synapses;
    for (int i = 0; i < kInputs; ++i) {
        sources.emplace_back(1000 + uint64_t(i), kRate);
        synapses.emplace_back(0.3f + 0.1f * float(i % 5));
    }
    Neuron neuron;
    Homeostasis homeostasis;

    // Spikes per 10 s bin while running `seconds` of simulated time.
    auto simulate = [&](int seconds, bool scaling) {
        std::vector<int> bins(size_t(seconds / 10), 0);
        const int steps_per_bin = int(10000.f / kStepMs);
        for (int b = 0; b < int(bins.size()); ++b)
            for (int i = 0; i < steps_per_bin; ++i) {
                for (int k = 0; k < kInputs; ++k)
                    if (sources[size_t(k)].step(kStepMs))
                        neuron.excite(synapses[size_t(k)].weight() * homeostasis.gain() * kMaxConductance);
                const bool fired = neuron.step(0.f);
                if (scaling) homeostasis.step(fired, kStepMs);
                bins[size_t(b)] += fired;
            }
        return bins;
    };
    auto window_rate = [](const std::vector<int>& bins, size_t end) {
        return float(bins[end - 2] + bins[end - 1]) / 20.f;
    };

    ScalingRun r;
    const auto warmup = simulate(100, true);
    r.before = window_rate(warmup, warmup.size());
    auto effective = [&](size_t k) { return synapses[k].weight() * homeostasis.gain(); };
    r.ratio_before = effective(4) / effective(0);
    for (auto& s : sources) s.set_rate(2.f * kRate);
    const auto after = simulate(60, keep_homeostasis);
    r.after = window_rate(after, 5);
    for (int c : after) r.rates.push_back(float(c) / 10.f);
    r.ratio_after = effective(4) / effective(0);
    return r;
}

void test_homeostatic_scaling() {
    const ScalingRun on = run_scaling(true), off = run_scaling(false);
    for (const auto* r : {&on, &off}) {
        std::printf("  %s: before %.2f Hz | after doubling, per 10 s:", r == &on ? "on " : "off", r->before);
        for (float x : r->rates) std::printf(" %.1f", x);
        std::printf(" | 30-50 s window %.2f Hz | weight ratio drift %.2e\n", r->after,
                    r->ratio_after / r->ratio_before - 1.f);
    }
    CHECK(on.before >= 4.f && on.before <= 6.f);
    CHECK(on.after >= 4.f && on.after <= 6.f);
    CHECK(off.after < 4.f || off.after > 6.f);
    CHECK(std::fabs(on.ratio_after / on.ratio_before - 1.f) < 1e-6f);
    CHECK(HomeostasisParams{}.tau == 5000.f);
    CHECK(HomeostasisParams{}.target_rate == 5.f);
}

// INB-01: an excitatory population with recurrent STDP synapses under a
// strong constant background, with and without generic interneurons that
// sample it and inhibit it back. Mean rate per 1 s window, in Hz.
std::vector<float> run_population(bool inhibition) {
    constexpr int kExcitatory = 200, kInhibitory = kExcitatory / 4;  // 4:1, as in cortex
    constexpr float kRecurrence = 0.1f;      // E->E connection probability
    constexpr float kRecurrentPeak = 12.f;   // nS at full weight; a small network needs strong synapses
    constexpr float kToInterneuron = 0.5f;   // E->I connection probability
    constexpr float kToInterneuronG = 1.f;   // nS per spike, fixed
    constexpr float kBackgroundRate = 4000.f, kBackgroundG = 0.6f;  // 200 inputs at 20 Hz, pooled
    Rng wiring(7);

    struct Connection { int pre, post; Synapse synapse; };
    std::vector<Connection> recurrent;
    std::vector<std::vector<int>> outgoing(kExcitatory), incoming(kExcitatory), to_interneurons(kExcitatory);
    for (int a = 0; a < kExcitatory; ++a)
        for (int b = 0; b < kExcitatory; ++b)
            if (a != b && wiring.uniform() < kRecurrence) {
                outgoing[size_t(a)].push_back(int(recurrent.size()));
                incoming[size_t(b)].push_back(int(recurrent.size()));
                recurrent.push_back({a, b, Synapse(0.3f)});
            }
    for (auto& targets : to_interneurons)
        for (int b = 0; b < kInhibitory; ++b)
            if (wiring.uniform() < kToInterneuron) targets.push_back(b);

    std::vector<Neuron> excitatory(kExcitatory);
    std::vector<Interneuron> interneurons(kInhibitory);  // each one inhibits the whole population
    std::vector<SpikeSource> background;
    for (int i = 0; i < kExcitatory; ++i) background.emplace_back(100 + uint64_t(i), kBackgroundRate);

    std::vector<float> rates;
    std::vector<int> fired;
    const int steps_per_window = int(1000.f / kStepMs);
    for (int window = 0; window < 30; ++window) {
        int spikes = 0;
        for (int i = 0; i < steps_per_window; ++i) {
            const float t = float(window * steps_per_window + i) * kStepMs;
            fired.clear();
            for (int k = 0; k < kExcitatory; ++k) {
                excitatory[size_t(k)].excite(kBackgroundG * float(background[size_t(k)].step(kStepMs)));
                if (excitatory[size_t(k)].step(0.f)) fired.push_back(k);
            }
            for (auto& cell : interneurons)
                if (cell.step(0.f) && inhibition)
                    for (auto& target : excitatory) cell.inhibit(target);
            for (int a : fired) {
                for (int c : outgoing[size_t(a)]) {
                    Connection& link = recurrent[size_t(c)];
                    link.synapse.apply(link.synapse.on_pre(t));
                    excitatory[size_t(link.post)].excite(link.synapse.weight() * kRecurrentPeak);
                }
                for (int c : incoming[size_t(a)]) {
                    Synapse& synapse = recurrent[size_t(c)].synapse;
                    synapse.apply(synapse.on_post(t));
                }
                for (int b : to_interneurons[size_t(a)]) interneurons[size_t(b)].excite(kToInterneuronG);
            }
            spikes += int(fired.size());
        }
        rates.push_back(float(spikes) / float(kExcitatory));
    }
    return rates;
}

float mean_rate(const std::vector<float>& rates, size_t from, size_t to) {
    float sum = 0.f;
    for (size_t i = from; i < to; ++i) sum += rates[i];
    return sum / float(to - from);
}

void test_inhibition() {
    const std::vector<float> with = run_population(true), without = run_population(false);
    for (const auto* r : {&with, &without}) {
        std::printf("  %s:", r == &with ? "with inhibition   " : "without inhibition");
        for (float x : *r) std::printf(" %.0f", x);
        std::printf(" Hz\n");
    }
    // Stable: over the last 10 s, neighbouring windows differ by less than 20%,
    // no trend from the 10 s before, and well below the 60 Hz instability limit.
    for (size_t i = 21; i < 30; ++i) CHECK(std::fabs(with[i] - with[i - 1]) < 0.2f * with[i - 1]);
    CHECK(std::fabs(mean_rate(with, 20, 30) / mean_rate(with, 10, 20) - 1.f) < 0.2f);
    CHECK(*std::max_element(with.begin() + 20, with.end()) < 60.f);
    // Without it, the same population grows past 60 Hz and stays there.
    CHECK(mean_rate(without, 20, 30) > 60.f);
    CHECK(mean_rate(without, 20, 30) > 2.f * mean_rate(without, 0, 2));

    // An interneuron spike lowers the target's potential; its synapse is fixed.
    Interneuron cell;
    const float fixed = cell.conductance();
    Neuron target;
    cell.inhibit(target);
    CHECK(potential_range(target, 100.f).first < AdExParams{}.rest - 0.2f);
    CHECK(cell.conductance() == fixed);
    CHECK(Interneuron().params().cell.spike_jump == 0.f);  // fast, non-adapting
}

// APR-01: one neuron with plastic input synapses learns a 50 ms pattern that
// half of its afferents repeat in 1/4 of the time, hidden in Poisson noise of
// the same density (Masquelier, Guyonneau & Thorpe 2008, reduced). Learning
// depends on the seed: this configuration passes every check for 5 of seeds
// 1-8 (docs/parametros.md); the others stay unselective or lose it later.
struct PatternRun {
    float ratio;                      // Hz inside the pattern / Hz outside, last 100 presentations
    float inside, outside;            // Hz
    std::vector<float> window_rates;  // Hz per 20 s window, whole run
};

PatternRun run_pattern(int presentations, bool plastic, uint64_t seed) {
    constexpr int kAfferents = 500, kSlotSteps = int(50.f / kStepMs);
    constexpr float kRate = 20.f, kPeak = 1.f;  // Hz per afferent; nS at full weight
    // Song et al. 2000 with Masquelier's shorter potentiation window and the
    // A+ of Song's Fig. 2f: A-tau-/A+tau+ = 1.25. All within docs/parametros.md.
    StdpParams stdp;
    stdp.potentiation = 0.02f;
    stdp.tau_plus = 16.8f;

    Rng rng(seed);
    std::vector<SpikeSource> noise;
    std::vector<Synapse> synapses;
    for (int i = 0; i < kAfferents; ++i) {
        noise.emplace_back(seed * 1000 + uint64_t(i), kRate);
        synapses.emplace_back(0.3f + 0.4f * rng.uniform(), stdp);
    }
    // The pattern: 50 ms of the same Poisson process, frozen, for the first half.
    std::vector<std::vector<int>> pattern(kSlotSteps);
    for (int i = 0; i < kAfferents / 2; ++i) {
        SpikeSource frozen(seed * 1000 + 500 + uint64_t(i), kRate);
        for (int s = 0; s < kSlotSteps; ++s)
            if (frozen.step(kStepMs)) pattern[size_t(s)].push_back(i);
    }
    Neuron neuron;
    Homeostasis homeostasis;

    // Noise is scheduled per slot, so the cost follows the spikes, not the steps.
    std::vector<double> next(kAfferents);
    for (int i = 0; i < kAfferents; ++i) next[size_t(i)] = noise[size_t(i)].interval();
    std::vector<std::vector<int>> arriving(kSlotSteps);
    auto deliver = [&](int i, Millis t) {
        Synapse& s = synapses[size_t(i)];
        const float change = s.on_pre(t);
        if (plastic) s.apply(change);
        neuron.excite(s.weight() * homeostasis.gain() * kPeak);
    };

    PatternRun r{};
    int shown = 0, spikes_in = 0, spikes_out = 0, slots_in = 0, slots_out = 0, window_spikes = 0;
    bool previous = false;
    long step = 0;
    const long window_steps = long(20000.f / kStepMs);
    while (shown < presentations) {
        // Never twice in a row; 1/3 of the remaining slots gives 1/4 overall.
        const bool is_pattern = !previous && rng.uniform() < 1.f / 3.f;
        previous = is_pattern;
        shown += is_pattern;

        const double slot_start = double(step) * kStepMs;
        for (auto& a : arriving) a.clear();
        for (int i = 0; i < kAfferents; ++i)
            for (double& n = next[size_t(i)]; n < slot_start + 50.0; n += noise[size_t(i)].interval())
                if (!(is_pattern && i < kAfferents / 2))
                    arriving[size_t(std::min(int((n - slot_start) / kStepMs), kSlotSteps - 1))].push_back(i);
        if (is_pattern)
            for (int s = 0; s < kSlotSteps; ++s)
                arriving[size_t(s)].insert(arriving[size_t(s)].end(), pattern[size_t(s)].begin(), pattern[size_t(s)].end());

        int fired_in_slot = 0;
        for (int s = 0; s < kSlotSteps; ++s, ++step) {
            const Millis t = Millis(step) * kStepMs;
            for (int i : arriving[size_t(s)]) deliver(i, t);
            const bool fired = neuron.step(0.f);
            homeostasis.step(fired, kStepMs);
            if (fired) {
                ++fired_in_slot;
                for (auto& syn : synapses) {
                    const float change = syn.on_post(t);
                    if (plastic) syn.apply(change);
                }
            }
            window_spikes += fired;
            if ((step + 1) % window_steps == 0) {
                r.window_rates.push_back(float(window_spikes) / 20.f);
                window_spikes = 0;
            }
        }
        if (shown > presentations - 100) {
            (is_pattern ? spikes_in : spikes_out) += fired_in_slot;
            ++(is_pattern ? slots_in : slots_out);
        }
    }
    r.inside = float(spikes_in) / (0.05f * float(slots_in));
    r.outside = float(spikes_out) / (0.05f * float(slots_out));
    r.ratio = r.outside > 0.f ? r.inside / r.outside : std::numeric_limits<float>::infinity();
    return r;
}

void test_pattern_learning() {
    constexpr uint64_t kSeed = 5;
    const PatternRun learned = run_pattern(500, true, kSeed), frozen = run_pattern(500, false, kSeed);
    const PatternRun longer = run_pattern(5000, true, kSeed);
    for (const auto* r : {&learned, &frozen, &longer})
        std::printf("  %s: inside %.1f Hz, outside %.2f Hz, ratio %.1f\n",
                    r == &learned ? "500 presentations " : r == &frozen ? "plasticity off    " : "5000 presentations",
                    r->inside, r->outside, r->ratio);
    // "Stays in the HOM-01 range": every 20 s window after the first 100 s.
    const auto settled = longer.window_rates.begin() + 5;
    const auto [low, high] = std::minmax_element(settled, longer.window_rates.end());
    std::printf("  long run, %zu windows of 20 s after 100 s: %.2f to %.2f Hz\n",
                size_t(longer.window_rates.end() - settled), *low, *high);
    CHECK(learned.ratio >= 5.f);
    CHECK(frozen.ratio < 5.f);
    CHECK(longer.ratio >= 5.f);
    CHECK(*low >= 4.f && *high <= 6.f);
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
    run("synaptic transmission", test_synaptic_transmission);
    run("homeostatic scaling", test_homeostatic_scaling);
    run("inhibition", test_inhibition);
    run("pattern learning", test_pattern_learning);
    run("engram roundtrip and corruption", test_engram_roundtrip_and_corruption);
    run("fork diverges but remembers", test_fork_diverges_but_remembers);
    run("capture", test_capture);
    std::printf(failures ? "%d check(s) failed\n" : "all passed\n", failures);
    return failures ? 1 : 0;
}
