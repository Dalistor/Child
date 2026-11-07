#include "cerebellum.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace child {

Cerebellum::Cerebellum(const CerebellumParams& params, uint32_t mossy, Rng& rng)
    : p_(params), mossy_(0), bias_(kOutputs, 0.f) {
    if (p_.inputs == 0 || p_.expansion == 0) throw std::runtime_error("cerebellum needs inputs and expansion above 0");
    grow(mossy, rng);
}

void Cerebellum::grow(uint32_t mossy, Rng& rng) {
    if (mossy <= mossy_) return;
    const uint32_t first_new = mossy_;
    const uint32_t added = mossy - mossy_;
    mossy_ = mossy;
    add_granules(added * p_.expansion, first_new, rng);
}

void Cerebellum::add_granules(uint32_t count, uint32_t first_new_mossy, Rng& rng) {
    for (uint32_t g = 0; g < count; ++g) {
        // The first input comes from the new fibres, so each one is read; the
        // others from any fibre, as granule cells mix sources.
        inputs_.push_back(first_new_mossy + uint32_t(rng.below(mossy_ - first_new_mossy)));
        for (uint32_t k = 1; k < p_.inputs; ++k) inputs_.push_back(uint32_t(rng.below(mossy_)));
    }
    weights_.resize(size_t(granules()) * kOutputs, 0.f);  // a new parallel fibre starts silent
}

void Cerebellum::encode(const std::vector<float>& mossy, std::vector<uint32_t>& active) const {
    const uint32_t n = granules();
    drive_.assign(n, 0.f);
    order_.clear();
    for (uint32_t g = 0; g < n; ++g) {
        float sum = 0.f;
        for (uint32_t k = 0; k < p_.inputs; ++k) sum += mossy[inputs_[size_t(g) * p_.inputs + k]];
        drive_[g] = sum;
        if (sum > 0.f) order_.push_back(g);
    }
    // Golgi inhibition as k-winners-take-all; ties go to the lower index, so the result is deterministic.
    const size_t k = std::min(order_.size(), size_t(std::max(1.f, p_.coding_level * float(n))));
    std::partial_sort(order_.begin(), order_.begin() + std::ptrdiff_t(k), order_.end(), [&](uint32_t a, uint32_t b) {
        return drive_[a] != drive_[b] ? drive_[a] > drive_[b] : a < b;
    });
    active.assign(order_.begin(), order_.begin() + std::ptrdiff_t(k));
    std::sort(active.begin(), active.end());
}

void Cerebellum::predict(const std::vector<uint32_t>& active, float* probs) const {
    std::copy(bias_.begin(), bias_.end(), probs);
    for (uint32_t g : active) {
        const float* w = &weights_[size_t(g) * kOutputs];
        for (uint32_t v = 0; v < kOutputs; ++v) probs[v] += w[v];
    }
    const float top = *std::max_element(probs, probs + kOutputs);
    double sum = 0.0;
    for (uint32_t v = 0; v < kOutputs; ++v) sum += (probs[v] = std::exp(probs[v] - top));
    for (uint32_t v = 0; v < kOutputs; ++v) probs[v] = float(double(probs[v]) / sum);
}

void Cerebellum::learn(const std::vector<uint32_t>& active, uint8_t target, const float* probs) {
    const float rate = p_.learning_rate / float(std::max<size_t>(1, active.size()));
    float error[kOutputs];  // what the climbing fibre of each unit reports
    for (uint32_t v = 0; v < kOutputs; ++v) {
        error[v] = rate * ((v == target ? 1.f : 0.f) - probs[v]);
        bias_[v] += error[v];
    }
    for (uint32_t g : active) {
        float* w = &weights_[size_t(g) * kOutputs];
        for (uint32_t v = 0; v < kOutputs; ++v) w[v] += error[v];
    }
}

}  // namespace child
