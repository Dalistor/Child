#pragma once

namespace child {

// Adaptive exponential integrate-and-fire neuron (Brette & Gerstner 2005).
// Units: mV, ms, pF, nS, pA. Defaults are the paper's fit to a regular-spiking
// pyramidal cell; ranges and sources are in docs/parametros.md.
struct AdExParams {
    float capacitance = 281.f;    // C, pF
    float leak = 30.f;            // g_L, nS
    float rest = -70.6f;          // E_L, mV
    float threshold = -50.4f;     // V_T, mV
    float slope = 2.f;            // Delta_T, mV
    float subthreshold = 4.f;     // a, nS: adaptation coupled to the potential
    float adaptation_tau = 144.f; // tau_w, ms
    float spike_jump = 80.5f;     // b, pA: adaptation added at each spike
    float reset = -70.6f;         // V_reset, mV
    float peak = 20.f;            // V_peak, mV: crossing it counts as a spike
};

// Integration step used by default, in ms.
constexpr float kStepMs = 0.1f;

class Neuron {
public:
    explicit Neuron(const AdExParams& params = {});

    // Advances `dt` ms under an input current in pA (forward Euler). Returns
    // true when the neuron fired during this step; it is then already reset.
    bool step(float current, float dt = kStepMs);

    float potential() const { return v_; }   // V, mV
    float adaptation() const { return w_; }  // w, pA
    const AdExParams& params() const { return p_; }

private:
    AdExParams p_;
    float v_;
    float w_ = 0.f;
};

}  // namespace child
