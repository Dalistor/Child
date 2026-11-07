#include "thalamus.hpp"

#include <cmath>
#include <limits>

namespace child {

SpikeSource::SpikeSource(uint64_t seed, float rate_hz) : rng_(seed), rate_(rate_hz), until_next_(interval()) {}

double SpikeSource::interval() {
    if (rate_ <= 0.f) return std::numeric_limits<double>::infinity();
    // Exponential interval; 1 - u lies in (0, 1], so the log is finite.
    return -std::log(1.0 - double(rng_.uniform())) * 1000.0 / double(rate_);
}

int SpikeSource::step(float dt) {
    until_next_ -= dt;
    int spikes = 0;
    for (; until_next_ <= 0.0; ++spikes) until_next_ += interval();
    return spikes;
}

}  // namespace child
