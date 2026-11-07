#include "brain.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <string>

#include "brain/archive.hpp"

namespace child {

namespace {

constexpr uint64_t kThalamusSeed = 0x7468616c616d7573ULL;  // "thalamus": its codes and cells get their own stream

uint32_t steps_per_byte(const BrainParams& p) { return uint32_t(std::lround(p.byte_ms / p.step_ms)); }

}  // namespace

Brain::Brain(const BrainParams& params, uint64_t seed)
    : p_(params),
      seed_(seed),
      rng_(seed),
      thalamus_(params.thalamus, seed ^ kThalamusSeed),
      cortex_(params.cortex, params.thalamus.channels, rng_),
      cerebellum_(params.cerebellum, params.thalamus.channels + cortex_.size(), rng_),
      hippocampus_(params.hippocampus),
      dopamine_(params.dopamine_tau),
      neurogenesis_(params.neurogenesis),
      expected_(std::log(256.f)),
      trace_(params.thalamus.channels + cortex_.size(), 0.f),
      relayed_(params.thalamus.channels, 0),
      probs_(kOutputs, 1.f / float(kOutputs)) {
    if (steps_per_byte(p_) == 0) throw std::runtime_error("brain.byte_ms must be at least one brain.step_ms");
}

float Brain::perceive(uint8_t byte, bool learn) {
    const float surprise = -std::log(std::max(probs_[byte], 1e-30f));
    released_ = 0.f;
    if (learn && has_prediction_) {
        // The prediction is checked against what came: the cerebellum learns
        // from its error, the hippocampus keeps the episode, and the part of
        // the surprise that was not expected is released as dopamine.
        cerebellum_.learn(active_, byte, probs_.data());
        hippocampus_.store(active_, byte, rng_);
        released_ = p_.dopamine_gain * std::max(0.f, surprise - expected_);
        if (released_ > 0.f) dopamine_.release(released_);
    }
    if (has_prediction_) expected_ += (surprise - expected_) / p_.expectation_bytes;

    thalamus_.relay(byte);
    cortex_.begin_byte();
    std::fill(relayed_.begin(), relayed_.end(), 0);
    const uint32_t steps = steps_per_byte(p_);
    for (uint32_t i = 0; i < steps; ++i) {
        relay_.clear();
        thalamus_.step(p_.step_ms, relay_);
        for (uint32_t c : relay_) ++relayed_[c];
        cortex_.step(time_, p_.step_ms, relay_, learn);
        dopamine_.step(p_.step_ms);
        dopamine_integral_ += dopamine_.level() * p_.step_ms;
        time_ += p_.step_ms;
    }
    // Reinforcing every synapse costs one pass over all of them, so it runs
    // once per interval with the dopamine integrated over it: dw = e * integral.
    if (++since_reinforce_ * p_.byte_ms >= p_.reinforce_ms) {
        if (learn) cortex_.reinforce(dopamine_integral_, time_);
        dopamine_integral_ = 0.f;
        since_reinforce_ = 0;
    }
    cortex_.end_byte(learn, rng_);
    read_out();

    if (learn)
        if (const uint32_t add = neurogenesis_.observe(surprise, cortex_.size())) grow(add);
    return surprise;
}

void Brain::read_out() {
    const float keep = std::exp(-p_.byte_ms / p_.context_tau);
    // Mossy fibres: the relay channels first, then every cortical cell.
    const auto& counts = cortex_.counts();
    const size_t channels = relayed_.size();
    for (size_t i = 0; i < trace_.size(); ++i)
        trace_[i] = keep * trace_[i] + (i < channels ? p_.sensory_weight * float(relayed_[i]) : float(counts[i - channels]));
    cerebellum_.encode(trace_, active_);
    cerebellum_.predict(active_, probs_.data());
    has_prediction_ = 1;
}

void Brain::sleep(uint32_t cycles) {
    std::vector<float> probs(kOutputs);
    for (uint32_t c = 0; c < cycles; ++c)
        hippocampus_.sleep(rng_, [&](const std::vector<uint32_t>& pattern, uint8_t next) {
            cerebellum_.predict(pattern, probs.data());
            cerebellum_.learn(pattern, next, probs.data());
        });
    if (has_prediction_) cerebellum_.predict(active_, probs_.data());
}

uint32_t Brain::grow(uint32_t n) {
    cortex_.grow(n, time_, rng_);
    trace_.resize(relayed_.size() + cortex_.size(), 0.f);
    cerebellum_.grow(uint32_t(trace_.size()), rng_);
    return n;
}

template <class Archive>
void Brain::persist(Archive& a) {
    rng_.persist(a);
    thalamus_.persist(a);
    cortex_.persist(a);
    cerebellum_.persist(a);
    hippocampus_.persist(a);
    dopamine_.persist(a);
    neurogenesis_.persist(a);
    a(time_);
    a(dopamine_integral_);
    a(since_reinforce_);
    a(expected_);
    a(released_);
    a(has_prediction_);
    a(trace_);
    a(relayed_);
    a(active_);
    a(probs_);
}

std::vector<uint8_t> Brain::serialize() const {
    ByteWriter w;
    const std::string text = describe(p_);
    w(seed_);
    w(std::vector<uint8_t>(text.begin(), text.end()));
    const_cast<Brain*>(this)->persist(w);  // persist() only reads when writing
    return std::move(w.bytes);
}

Brain Brain::deserialize(const std::vector<uint8_t>& bytes) {
    ByteReader r(bytes.data(), bytes.size());
    uint64_t seed = 0;
    std::vector<uint8_t> text;
    r(seed);
    r(text);
    BrainParams params;
    configure(params, std::string(text.begin(), text.end()));
    Brain brain(params, seed);
    brain.persist(r);
    if (r.remaining() != 0) throw std::runtime_error("brain section has trailing data");
    return brain;
}

}  // namespace child
