#pragma once

namespace child {

// Homeostatic synaptic scaling (Turrigiano 2008), with the fast time scale
// that keeps STDP networks stable in models (Zenke et al. 2013). Ranges and
// sources are in docs/parametros.md.
struct HomeostasisParams {
    float target_rate = 5.f;     // Hz
    float tau = 5000.f;          // tau_hom, ms: how long the rate sensor integrates
    float learning_rate = 1e-4f; // 1/ms: relative weight change per ms at zero rate
};

// One per neuron. It senses the neuron's firing rate and keeps a gain that
// multiplies every incoming weight: the effective weight is weight * gain.
// One shared factor keeps the proportions STDP has learned exactly, and
// costs one multiplication per step instead of one per synapse.
class Homeostasis {
public:
    explicit Homeostasis(const HomeostasisParams& params = {});

    // Advances the rate sensor by `dt` ms and scales the gain: up when the
    // neuron fires below the target, down when above.
    void step(bool fired, float dt);

    float rate() const;  // Hz
    float gain() const { return gain_; }
    const HomeostasisParams& params() const { return p_; }

private:
    HomeostasisParams p_;
    float rate_;  // spikes per ms, starts at the target
    float gain_ = 1.f;
};

}  // namespace child
