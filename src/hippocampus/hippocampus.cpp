#include "hippocampus.hpp"

#include <algorithm>
#include <iterator>

namespace child {

Hippocampus::Hippocampus(const HippocampusParams& params) : p_(params), start_{0} {}

void Hippocampus::store(const std::vector<uint32_t>& pattern, uint8_t next, Rng& rng) {
    ++seen_;
    if (p_.capacity == 0) return;
    if (size() < p_.capacity) {
        put(size(), pattern, next);
        return;
    }
    // Reservoir sampling: the n-th episode replaces a random one with probability capacity / n.
    const uint64_t slot = rng.below(seen_);
    if (slot < p_.capacity) put(uint32_t(slot), pattern, next);
}

void Hippocampus::put(uint32_t slot, const std::vector<uint32_t>& pattern, uint8_t next) {
    if (slot == size()) {
        cells_.insert(cells_.end(), pattern.begin(), pattern.end());
        start_.push_back(uint32_t(cells_.size()));
        next_.push_back(next);
        return;
    }
    // Replace in place, shifting the later episodes when the length differs.
    const auto from = std::ptrdiff_t(start_[slot]), to = std::ptrdiff_t(start_[slot + 1]);
    const auto delta = std::ptrdiff_t(pattern.size()) - (to - from);
    cells_.erase(cells_.begin() + from, cells_.begin() + to);
    cells_.insert(cells_.begin() + from, pattern.begin(), pattern.end());
    for (size_t i = slot + 1; i < start_.size(); ++i) start_[i] = uint32_t(std::ptrdiff_t(start_[i]) + delta);
    next_[slot] = next;
}

std::vector<uint32_t> Hippocampus::pattern(uint32_t episode) const {
    return {cells_.begin() + start_[episode], cells_.begin() + start_[episode + 1]};
}

int64_t Hippocampus::recall(const std::vector<uint32_t>& cue) const {
    int64_t best = -1;
    size_t best_overlap = 0;
    std::vector<uint32_t> shared;
    for (uint32_t e = 0; e < size(); ++e) {
        shared.clear();
        std::set_intersection(cells_.begin() + start_[e], cells_.begin() + start_[e + 1], cue.begin(), cue.end(),
                              std::back_inserter(shared));
        if (shared.size() > best_overlap) best = e, best_overlap = shared.size();
    }
    return best;
}

}  // namespace child
