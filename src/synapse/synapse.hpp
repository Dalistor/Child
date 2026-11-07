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

// Eligibility trace for three-factor learning (Fremaux & Gerstner 2016): STDP
// changes wait here until dopamine turns them into weight, or until they expire.
struct EligibilityParams {
    float tau = 1000.f;      // tau_e, ms
    float lifetime = 5000.f; // ms after the last tag; then the trace is exactly zero
};

// Spike times in ms. Double precision keeps the STDP window exact over long
// runs: in float32 a time of 1e7 ms (under 3 h) already has 1 ms resolution.
using Millis = double;

// A synapse only sees spike events and their times. STDP proposes a change at
// each event and apply() commits it, so a third factor (dopamine) can sit in
// between later without touching the rule.
class Synapse {
public:
    explicit Synapse(float weight, const StdpParams& params = {}, const EligibilityParams& eligibility = {});

    // Spike times must not decrease. Each call returns the weight change the
    // STDP rule proposes for that event; nothing is applied yet.
    float on_pre(Millis t);
    float on_post(Millis t);

    // Adds `change` and clamps the weight to [0, max_weight].
    void apply(float change);

    // Three-factor path: tag() keeps a proposed change as eligibility at time
    // t instead of applying it; reinforce() turns eligibility into weight at
    // the rate of the dopamine level, dw = e * dopamine * dt.
    void tag(float change, Millis t);
    void reinforce(float dopamine, Millis t, float dt);
    float eligibility(Millis t) const;

    float weight() const { return w_; }
    const StdpParams& params() const { return p_; }

private:
    StdpParams p_;
    EligibilityParams e_params_;
    float w_;
    // Exponentially decaying traces of past spikes, decayed lazily to the
    // time of the next event.
    float pre_trace_ = 0.f, post_trace_ = 0.f;
    Millis pre_time_ = 0, post_time_ = 0;
    float tag_ = 0.f;  // eligibility at the last tag
    Millis tag_time_ = 0;
};

}  // namespace child
