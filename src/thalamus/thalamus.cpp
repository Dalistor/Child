#include "thalamus.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <limits>

namespace child {

SpikeSource::SpikeSource(uint64_t seed, float rate_hz) : rng_(seed), rate_(rate_hz), until_next_(interval()) {}

double SpikeSource::interval() {
    if (rate_ <= 0.f) return std::numeric_limits<double>::infinity();
    // Exponential interval; 1 - u lies in (0, 1], so the log is finite.
    return -std::log(1.0 - double(rng_.uniform())) * 1000.0 / double(rate_);
}

void SpikeSource::set_rate(float rate_hz) {
    rate_ = rate_hz;
    until_next_ = interval();
}

int SpikeSource::step(float dt) {
    until_next_ -= dt;
    int spikes = 0;
    for (; until_next_ <= 0.0; ++spikes) until_next_ += interval();
    return spikes;
}

Thalamus::Thalamus(const ThalamusParams& params, uint64_t seed) : p_(params), codes_(256) {
    if (p_.channels_per_byte == 0 || p_.channels_per_byte > p_.channels)
        throw std::runtime_error("thalamus.channels_per_byte must be between 1 and thalamus.channels");
    Rng rng(seed);
    for (auto& code : codes_) {
        while (code.size() < p_.channels_per_byte) {
            const auto c = uint32_t(rng.below(p_.channels));
            if (std::find(code.begin(), code.end(), c) == code.end()) code.push_back(c);
        }
        std::sort(code.begin(), code.end());
    }
    for (uint32_t c = 0; c < p_.channels; ++c) cells_.emplace_back(rng.next(), p_.spontaneous_rate);
}

void Thalamus::relay(uint8_t byte) {
    if (current_ == byte) return;
    if (current_ >= 0)
        for (uint32_t c : codes_[size_t(current_)]) cells_[c].set_rate(p_.spontaneous_rate);
    for (uint32_t c : codes_[byte]) cells_[c].set_rate(p_.active_rate);
    current_ = byte;
}

void Thalamus::step(float dt, std::vector<uint32_t>& fired) {
    for (uint32_t c = 0; c < cells_.size(); ++c)
        for (int n = cells_[c].step(dt); n > 0; --n) fired.push_back(c);
}

}  // namespace child
