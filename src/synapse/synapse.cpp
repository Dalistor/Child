#include "synapse.hpp"

#include <algorithm>
#include <cmath>

namespace child {

namespace {

float decayed(float trace, float since, float tau) { return trace * std::exp(-since / tau); }

}  // namespace

Synapse::Synapse(float weight, const StdpParams& params, const EligibilityParams& eligibility)
    : p_(params), e_params_(eligibility), w_(weight) {}

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

void Synapse::tag(float change, float t) {
    tag_ = eligibility(t) + change;
    tag_time_ = t;
}

void Synapse::reinforce(float dopamine, float t, float dt) {
    // Without dopamine, or with an expired trace, the weight is left exactly as is.
    const float e = eligibility(t);
    if (dopamine == 0.f || e == 0.f) return;
    apply(e * dopamine * dt);
}

float Synapse::eligibility(float t) const {
    const float since = t - tag_time_;
    if (since >= e_params_.lifetime) return 0.f;
    return decayed(tag_, since, e_params_.tau);
}

}  // namespace child
