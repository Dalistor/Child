#include "neuron.hpp"

#include <cmath>

namespace child {

Neuron::Neuron(const AdExParams& params, const ReceptorParams& receptors)
    : p_(params), r_(receptors), v_(params.rest) {}

bool Neuron::step(float current, float dt) {
    // C dV/dt = -g_L (V - E_L) + g_L Delta_T exp((V - V_T) / Delta_T) - w + I + I_syn
    // tau_w dw/dt = a (V - E_L) - w
    // I_syn = g_E (E_E - V) + g_I (E_I - V); each g decays with its own tau.
    const float synaptic = g_e_ * (r_.excitatory_reversal - v_) + g_i_ * (r_.inhibitory_reversal - v_);
    const float upswing = p_.leak * p_.slope * std::exp((v_ - p_.threshold) / p_.slope);
    const float dv = (-p_.leak * (v_ - p_.rest) + upswing - w_ + current + synaptic) / p_.capacitance;
    const float dw = (p_.subthreshold * (v_ - p_.rest) - w_) / p_.adaptation_tau;
    v_ += dt * dv;
    w_ += dt * dw;
    g_e_ -= dt * g_e_ / r_.excitatory_tau;
    g_i_ -= dt * g_i_ / r_.inhibitory_tau;

    // Once the exponential takes over, V diverges within a step; the crossing
    // of V_peak is the spike, and the potential never stays above it.
    if (v_ < p_.peak) return false;
    v_ = p_.reset;
    w_ += p_.spike_jump;
    return true;
}

}  // namespace child
