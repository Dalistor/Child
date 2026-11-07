#pragma once

#include <cstdint>
#include <vector>

#include "model/rng.hpp"

namespace child {

// The fast learner of the complementary learning systems (McClelland,
// McNaughton & O'Reilly 1995). It keeps episodes in one shot: the sparse
// pattern that preceded a byte and the byte itself. The store holds a uniform
// sample of everything experienced (reservoir sampling), so old experience
// stays represented as new experience arrives. During sleep the episodes are
// replayed, interleaved and shuffled, to the slow learner.
struct HippocampusParams {
    uint32_t capacity = 2048;  // episodes kept
    uint32_t replay = 1;       // passes over the store per sleep cycle
};

class Hippocampus {
public:
    Hippocampus(const HippocampusParams& params);

    // One episode: the active pattern (sorted indices) and what followed it.
    void store(const std::vector<uint32_t>& pattern, uint8_t next, Rng& rng);

    // Calls `replay(pattern, next)` for every stored episode, in an order
    // drawn from `rng`, `params().replay` times.
    template <class Replay>
    void sleep(Rng& rng, Replay replay) const {
        std::vector<uint32_t> order(size());
        for (uint32_t pass = 0; pass < p_.replay; ++pass) {
            for (uint32_t i = 0; i < order.size(); ++i) order[i] = i;
            for (size_t i = order.size(); i > 1; --i) std::swap(order[i - 1], order[size_t(rng.below(i))]);
            for (uint32_t e : order) replay(pattern(e), next_[e]);
        }
    }

    // The stored episode whose pattern overlaps `cue` the most, as completion
    // from a partial cue (CA3); -1 when nothing overlaps.
    int64_t recall(const std::vector<uint32_t>& cue) const;

    uint32_t size() const { return uint32_t(next_.size()); }
    uint64_t seen() const { return seen_; }
    std::vector<uint32_t> pattern(uint32_t episode) const;
    uint8_t next(uint32_t episode) const { return next_[episode]; }
    const HippocampusParams& params() const { return p_; }

    template <class Archive>
    void persist(Archive& a) {
        a(seen_);
        a(start_);
        a(cells_);
        a(next_);
    }

private:
    void put(uint32_t slot, const std::vector<uint32_t>& pattern, uint8_t next);

    HippocampusParams p_;
    uint64_t seen_ = 0;
    std::vector<uint32_t> start_;  // offsets into cells_, size() + 1 entries
    std::vector<uint32_t> cells_;
    std::vector<uint8_t> next_;
};

}  // namespace child
