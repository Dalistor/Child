#include "cortex.hpp"

#include <algorithm>
#include <cmath>

namespace child {

namespace {
constexpr float kNegligible = 1e-7f;
}  // namespace

Cortex::Cortex(const CortexParams& params, uint32_t channels, Rng& rng)
    : p_(params), channels_(channels), to_interneuron_start_{0}, inhibitor_start_{0} {
    interneurons_.assign(p_.interneurons, Interneuron(p_.interneuron));
    for (uint32_t cell = 0; cell < p_.excitatory; ++cell) wire(-1e12, rng);  // born long ago: not young
    // Segments sample the whole first population, so they are drawn once it exists.
    for (uint32_t cell = 0; cell < size(); ++cell)
        for (uint32_t k = 0; k < p_.segments; ++k)
            segments_.emplace_back(p_.dendrite.synapses, size(), p_.dendrite.initial, rng);
    index_inhibition();
}

void Cortex::index_inhibition() {
    inhibits_.assign(interneurons(), {});
    for (uint32_t cell = 0; cell < size(); ++cell)
        for (uint32_t k = inhibitor_start_[cell]; k < inhibitor_start_[cell + 1]; ++k) inhibits_[inhibitor_[k]].push_back(cell);
}

void Cortex::wire(Millis now, Rng& rng) {
    pyramidal_.emplace_back(p_.neuron);
    homeostasis_.emplace_back(p_.homeostasis);
    for (uint32_t c = 0; c < channels_; ++c) {
        const float w = p_.initial_weight + p_.initial_spread * (2.f * rng.uniform() - 1.f);
        relay_.emplace_back(std::clamp(w, 0.f, p_.synapse.max_weight), p_.synapse, p_.eligibility);
    }
    // Each interneuron samples about to_interneuron_inputs of the first population.
    const float sampled = std::min(1.f, float(p_.to_interneuron_inputs) / float(std::max(1u, p_.excitatory)));
    for (uint32_t i = 0; i < interneurons(); ++i)
        if (rng.uniform() < sampled) to_interneuron_.push_back(i);
    to_interneuron_start_.push_back(uint32_t(to_interneuron_.size()));
    // And from_interneuron_inputs distinct interneurons inhibit the cell (all of them in a small cortex).
    const size_t from = inhibitor_.size();
    if (p_.from_interneuron_inputs >= interneurons()) {
        for (uint32_t i = 0; i < interneurons(); ++i) inhibitor_.push_back(i);
    } else {
        while (inhibitor_.size() - from < p_.from_interneuron_inputs) {
            const auto i = uint32_t(rng.below(interneurons()));
            if (std::find(inhibitor_.begin() + std::ptrdiff_t(from), inhibitor_.end(), i) == inhibitor_.end()) inhibitor_.push_back(i);
        }
    }
    inhibitor_start_.push_back(uint32_t(inhibitor_.size()));
    birth_.push_back(now);
    counts_.push_back(0);
    context_.push_back(0);
    predicted_.push_back(0);
}

void Cortex::resize(uint32_t n) {
    pyramidal_.assign(n, Neuron(p_.neuron));
    homeostasis_.assign(n, Homeostasis(p_.homeostasis));
    relay_.assign(size_t(n) * channels_, Synapse(0.f, p_.synapse, p_.eligibility));

    segments_.assign(size_t(n) * p_.segments, Segment());
    interneurons_.assign(p_.interneurons, Interneuron(p_.interneuron));
}

float Cortex::boost(uint32_t cell, Millis t) const {
    return t - birth_[cell] < p_.young_ms ? p_.young_boost : 1.f;
}

void Cortex::tag(size_t index, float change, uint32_t cell, Millis t) {
    // Pairs with a spike long gone propose changes far below what a weight can
    // resolve; skipping them leaves most synapses untagged, which reinforce() skips cheaply.
    if (std::fabs(change) < kNegligible) return;
    relay_[index].tag(change * boost(cell, t), t);
}

void Cortex::begin_byte() {
    for (uint32_t cell = 0; cell < size(); ++cell) {
        uint8_t predicted = 0;
        for (uint32_t k = 0; k < p_.segments && !predicted; ++k)
            predicted = segments_[size_t(cell) * p_.segments + k].overlap(context_, p_.dendrite.connected) >=
                        p_.dendrite.threshold;
        predicted_[cell] = predicted;
        counts_[cell] = 0;
    }
}

void Cortex::step(Millis t, float dt, const std::vector<uint32_t>& relay, bool learn) {
    const uint32_t n = size();
    for (uint32_t c : relay)
        for (uint32_t cell = 0; cell < n; ++cell) {
            const size_t index = size_t(cell) * channels_ + c;
            Synapse& s = relay_[index];
            const float change = s.on_pre(t);
            if (learn) tag(index, change, cell, t);
            pyramidal_[cell].excite(s.weight() * homeostasis_[cell].gain() * p_.thalamic_peak);
        }

    fired_.clear();
    for (uint32_t cell = 0; cell < n; ++cell) {
        const bool fired = pyramidal_[cell].step(predicted_[cell] ? p_.dendrite.plateau_current : 0.f, dt);
        if (learn) homeostasis_[cell].step(fired, dt);
        if (!fired) continue;
        fired_.push_back(cell);
        if (counts_[cell] < 255) ++counts_[cell];
    }
    for (uint32_t cell : fired_) {
        for (uint32_t c = 0; c < channels_; ++c) {
            const size_t index = size_t(cell) * channels_ + c;
            const float change = relay_[index].on_post(t);
            if (learn) tag(index, change, cell, t);
        }
        for (uint32_t k = to_interneuron_start_[cell]; k < to_interneuron_start_[cell + 1]; ++k)
            interneurons_[to_interneuron_[k]].excite(p_.to_interneuron_conductance);
    }
    for (uint32_t i = 0; i < interneurons(); ++i)
        if (interneurons_[i].step(0.f, dt))
            for (uint32_t cell : inhibits_[i]) interneurons_[i].inhibit(pyramidal_[cell]);
}

void Cortex::end_byte(bool learn, Rng& rng) {
    if (learn)
        for (uint32_t cell = 0; cell < size(); ++cell) {
            if (!counts_[cell]) continue;
            // The segment closest to the context learns it (Hawkins & Ahmad 2016).
            Segment* best = nullptr;
            uint32_t best_potential = 0;
            for (uint32_t k = 0; k < p_.segments; ++k) {
                Segment& s = segments_[size_t(cell) * p_.segments + k];
                const uint32_t potential = s.potential(context_);
                if (potential > best_potential) best = &s, best_potential = potential;
            }
            if (best) best->learn(context_, p_.dendrite, rng);
        }
    for (uint32_t cell = 0; cell < size(); ++cell) context_[cell] = counts_[cell] > 0;
}

void Cortex::reinforce(float dopamine, Millis t) {
    if (dopamine == 0.f) return;
    for (auto& s : relay_) s.reinforce(dopamine, t, 1.f);
}

void Cortex::grow(uint32_t n, Millis now, Rng& rng) {
    for (uint32_t i = 0; i < n; ++i) {
        wire(now, rng);
        for (uint32_t k = 0; k < p_.segments; ++k)
            segments_.emplace_back(p_.dendrite.synapses, size(), p_.dendrite.initial, rng);
    }
    index_inhibition();
}

uint32_t Cortex::predicted() const {
    return uint32_t(std::count(predicted_.begin(), predicted_.end(), uint8_t(1)));
}

float Cortex::mean_gain() const {
    float sum = 0.f;
    for (const auto& h : homeostasis_) sum += h.gain();
    return homeostasis_.empty() ? 0.f : sum / float(homeostasis_.size());
}

}  // namespace child
