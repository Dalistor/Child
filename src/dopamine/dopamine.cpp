#include "dopamine.hpp"

namespace child {

Dopamine::Dopamine(float tau) : tau_(tau) {}

void Dopamine::release(float amount) { level_ += amount / tau_; }

void Dopamine::step(float dt) { level_ -= dt * level_ / tau_; }

}  // namespace child
