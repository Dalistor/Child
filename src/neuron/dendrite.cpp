#include "dendrite.hpp"

#include <algorithm>
#include <cmath>

namespace child {

float calcium_amplitude(float drive, const CalciumParams& p) {
    if (drive < p.threshold) return 0.f;
    return std::exp(-(drive - p.threshold) / (p.decay * p.threshold));
}

Segment::Segment(uint32_t synapses, uint32_t population, float initial, Rng& rng) {
    for (uint32_t i = 0; i < synapses && population > 0; ++i) {
        pre_.push_back(uint32_t(rng.below(population)));
        permanence_.push_back(initial);
    }
}

uint32_t Segment::overlap(const std::vector<uint8_t>& active, float connected) const {
    uint32_t n = 0;
    for (size_t i = 0; i < pre_.size(); ++i) n += permanence_[i] >= connected && active[pre_[i]];
    return n;
}

uint32_t Segment::potential(const std::vector<uint8_t>& active) const {
    uint32_t n = 0;
    for (uint32_t p : pre_) n += active[p] != 0;
    return n;
}

void Segment::learn(const std::vector<uint8_t>& active, const SegmentParams& p, Rng& rng) {
    size_t weakest = 0;
    for (size_t i = 0; i < pre_.size(); ++i) {
        float& perm = permanence_[i];
        perm = std::clamp(perm + (active[pre_[i]] ? p.increment : -p.decrement), 0.f, 1.f);
        if (perm < permanence_[weakest]) weakest = i;
    }
    if (pre_.empty() || permanence_[weakest] >= p.connected) return;
    // Move the weakest synapse to a random active cell that it does not reach yet.
    std::vector<uint32_t> candidates;
    for (uint32_t c = 0; c < active.size(); ++c)
        if (active[c] && std::find(pre_.begin(), pre_.end(), c) == pre_.end()) candidates.push_back(c);
    if (candidates.empty()) return;
    pre_[weakest] = candidates[size_t(rng.below(candidates.size()))];
    permanence_[weakest] = p.initial;
}

}  // namespace child
