#pragma once

namespace child {

// Pair-based STDP (Song, Miller & Abbott 2000). Times in ms; amplitudes are
// fractions of the maximum weight per spike pair. Defaults and ranges are in
// docs/parametros.md.
struct StdpParams {
    float max_weight = 1.f;
    float potentiation = 0.005f;     // A+: pre shortly before post
    float depression_ratio = 1.05f;  // A-/A+: slightly above 1, so depression dominates
    float tau_plus = 20.f;           // ms, window for pre-before-post
    float tau_minus = 20.f;          // ms, window for post-before-pre
};

// A synapse only sees spike events and their times. STDP proposes a change at
// each event and apply() commits it, so a third factor (dopamine) can sit in
// between later without touching the rule.
class Synapse {
public:
    explicit Synapse(float weight, const StdpParams& params = {});

    // Spike times must not decrease. Each call returns the weight change the
    // STDP rule proposes for that event; nothing is applied yet.
    float on_pre(float t);
    float on_post(float t);

    // Adds `change` and clamps the weight to [0, max_weight].
    void apply(float change);

    float weight() const { return w_; }
    const StdpParams& params() const { return p_; }

private:
    StdpParams p_;
    float w_;
    // Exponentially decaying traces of past spikes, decayed lazily to the
    // time of the next event.
    float pre_trace_ = 0.f, post_trace_ = 0.f;
    float pre_time_ = 0.f, post_time_ = 0.f;
};

}  // namespace child
