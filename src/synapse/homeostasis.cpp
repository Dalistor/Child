#include "homeostasis.hpp"

namespace child {

Homeostasis::Homeostasis(const HomeostasisParams& params) : p_(params), rate_(params.target_rate / 1000.f) {}

void Homeostasis::step(bool fired, float dt) {
    // Exponential filter of the spike train: d rate/dt = -rate/tau + spikes/tau.
    rate_ -= dt * rate_ / p_.tau;
    if (fired) rate_ += 1.f / p_.tau;
    gain_ *= 1.f + p_.learning_rate * dt * (1.f - rate() / p_.target_rate);
}

float Homeostasis::rate() const { return rate_ * 1000.f; }

}  // namespace child
