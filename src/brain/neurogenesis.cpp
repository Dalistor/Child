#include "neurogenesis.hpp"

#include <algorithm>

namespace child {

uint32_t Neurogenesis::observe(float surprise, uint32_t neurons) {
    sum_ += surprise;
    if (++count_ < p_.window) return 0;
    const auto mean = float(sum_ / double(count_));
    const bool stalled = previous_ >= 0.f && mean > p_.min_surprise && mean > previous_ * (1.f - p_.min_improvement);
    previous_ = mean;
    sum_ = 0;
    count_ = 0;
    if (!p_.enabled || !stalled || neurons >= p_.max_neurons) return 0;
    const uint32_t add = std::min(p_.batch, p_.max_neurons - neurons);
    born_ += add;
    return add;
}

}  // namespace child
