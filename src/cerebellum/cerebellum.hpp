#pragma once

#include <cstdint>
#include <vector>

#include "model/rng.hpp"

namespace child {

// Marr-Albus-Ito readout of the next byte (docs/pesquisa-degraus.md §3).
// Mossy fibres carry the cortical activity; each granule cell sums a few of
// them and Golgi inhibition keeps only the most driven ones active (a sparse
// expansion). Purkinje-like units read the active granules; the climbing
// fibre brings the error of each unit, and the parallel-fibre synapse changes
// by the local delta rule: dw = rate * (target - p) for active granules only.
struct CerebellumParams {
    uint32_t expansion = 16;        // granule cells per mossy fibre (Litwin-Kumar 2017: ~30)
    uint32_t inputs = 4;            // mossy fibres per granule cell (Billings 2014: ~4)
    float coding_level = 0.05f;     // fraction of granule cells active
    float learning_rate = 0.5f;     // delta rule, divided by the number of active granules
};

constexpr uint32_t kOutputs = 256;  // one unit per byte value

class Cerebellum {
public:
    Cerebellum(const CerebellumParams& params, uint32_t mossy, Rng& rng);

    // Active granule cells for the given mossy activity, sorted.
    void encode(const std::vector<float>& mossy, std::vector<uint32_t>& active) const;
    // Probability of each next byte (kOutputs floats) from active granule cells.
    void predict(const std::vector<uint32_t>& active, float* probs) const;
    // Delta rule toward `target`, given the probabilities predict() returned.
    void learn(const std::vector<uint32_t>& active, uint8_t target, const float* probs);
    // More mossy fibres arrived (new cortical cells): adds their granule cells.
    void grow(uint32_t mossy, Rng& rng);

    uint32_t granules() const { return uint32_t(inputs_.size() / p_.inputs); }
    uint32_t mossy() const { return mossy_; }

    template <class Archive>
    void persist(Archive& a) {
        a(mossy_);
        a(inputs_);
        a(weights_);
        a(bias_);
    }

private:
    void add_granules(uint32_t count, uint32_t first_new_mossy, Rng& rng);

    CerebellumParams p_;
    uint32_t mossy_;
    std::vector<uint32_t> inputs_;  // [granule * inputs + k]: mossy index
    std::vector<float> weights_;    // [granule * kOutputs + v]: parallel fibre -> Purkinje
    std::vector<float> bias_;       // kOutputs
    mutable std::vector<float> drive_;  // scratch
    mutable std::vector<uint32_t> order_;
};

}  // namespace child
