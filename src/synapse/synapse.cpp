#include "synapse.hpp"

#include <algorithm>
#include <cmath>

namespace child {

namespace {

float decayed(float trace, float since, float tau) { return trace * std::exp(-since / tau); }

}  // namespace

Synapse::Synapse(float weight, const StdpParams& params) : p_(params), w_(weight) {}

float Synapse::on_pre(float t) {
    // A pre spike after recent post spikes depresses, in proportion to how
    // recent they were.
    const float post = decayed(post_trace_, t - post_time_, p_.tau_minus);
    pre_trace_ = decayed(pre_trace_, t - pre_time_, p_.tau_plus) + 1.f;
    pre_time_ = t;
    return -p_.depression_ratio * p_.potentiation * p_.max_weight * post;
}

float Synapse::on_post(float t) {
    // A post spike after recent pre spikes potentiates.
    const float pre = decayed(pre_trace_, t - pre_time_, p_.tau_plus);
    post_trace_ = decayed(post_trace_, t - post_time_, p_.tau_minus) + 1.f;
    post_time_ = t;
    return p_.potentiation * p_.max_weight * pre;
}

void Synapse::apply(float change) { w_ = std::clamp(w_ + change, 0.f, p_.max_weight); }

}  // namespace child
