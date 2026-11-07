#pragma once

#include <cstdint>

namespace child {

// When to create neurons (docs/pesquisa-degraus.md §5). Like the outlier rule
// of Draelos et al. 2017 and the error-driven insertion of Fritzke 1995: the
// brain watches its surprise in windows of bytes, and when a window stays
// surprising and no longer improves on the previous one, the cortex has run
// out of room, so a batch of new cells is born, up to a ceiling.
struct NeurogenesisParams {
    uint32_t enabled = 1;           // 0 turns it off (CHILD_NEUROGENESIS=off does it per run)
    uint32_t max_neurons = 1024;    // ceiling of pyramidal cells (CHILD_MAX_NEURONS overrides it per run)
    uint32_t window = 2048;         // bytes per window
    float min_surprise = 1.5f;      // nats: below it the cortex is coping
    float min_improvement = 0.02f;  // relative drop from the previous window that counts as progress
    uint32_t batch = 32;            // cells born at a time
};

class Neurogenesis {
public:
    explicit Neurogenesis(const NeurogenesisParams& params) : p_(params) {}

    // Records one byte's surprise; returns how many cells to add now (usually 0).
    uint32_t observe(float surprise, uint32_t neurons);

    const NeurogenesisParams& params() const { return p_; }
    NeurogenesisParams& params() { return p_; }
    uint64_t born() const { return born_; }

    template <class Archive>
    void persist(Archive& a) {
        a(sum_);
        a(count_);
        a(previous_);
        a(born_);
    }

private:
    NeurogenesisParams p_;
    double sum_ = 0;
    uint32_t count_ = 0;
    float previous_ = -1.f;  // mean surprise of the last window, -1 before the first
    uint64_t born_ = 0;
};

}  // namespace child
