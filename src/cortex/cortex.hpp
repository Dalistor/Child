#pragma once

#include <cstdint>
#include <vector>

#include "interneuron/interneuron.hpp"
#include "model/rng.hpp"
#include "neuron/dendrite.hpp"
#include "neuron/neuron.hpp"
#include "synapse/homeostasis.hpp"
#include "synapse/synapse.hpp"

namespace child {

// A patch of neocortex that reads the thalamus. Pyramidal cells (AdEx) receive
// every relay channel through three-factor STDP synapses scaled by their own
// homeostasis; distal segments listen to the cells active one byte earlier and
// depolarize the soma when they recognize that context; generic interneurons
// sample the pyramidal cells and inhibit them back. Fan-in and fan-out are
// fixed counts, not probabilities, so a large cortex stays as sparse as a
// small one (Vogels et al. 2011 wire 2% of 10,000 cells). New cells can be added
// at any time (neurogenesis); young cells learn faster for a while (Ge 2007).
struct CortexParams {
    uint32_t excitatory = 256;
    uint32_t interneurons = 64;             // 4:1, as in cortex
    float thalamic_peak = 2.f;              // nS per relay spike at full weight
    float initial_weight = 0.5f;            // mean of the uniform initial weights
    float initial_spread = 0.3f;            // half-width of that range
    uint32_t to_interneuron_inputs = 128;   // pyramidal cells each interneuron samples, on average
    uint32_t from_interneuron_inputs = 64;  // interneurons inhibiting each pyramidal cell
    float to_interneuron_conductance = 1.f; // nS per pyramidal spike
    uint32_t segments = 2;                  // distal segments per pyramidal cell
    float young_boost = 2.f;                // plasticity factor of a young cell
    float young_ms = 20000.f;               // how long a cell stays young

    AdExParams neuron;
    StdpParams synapse;
    EligibilityParams eligibility;
    HomeostasisParams homeostasis;
    InterneuronParams interneuron;
    SegmentParams dendrite;
};

class Cortex {
public:
    Cortex(const CortexParams& params, uint32_t channels, Rng& rng);

    // A byte starts: segments that recognize the previous byte's cells put
    // their neuron in the predicted (depolarized) state.
    void begin_byte();
    // One integration step with the relay spikes of that step.
    void step(Millis t, float dt, const std::vector<uint32_t>& relay, bool learn);
    // The byte ends: segments learn and the active set becomes the context.
    void end_byte(bool learn, Rng& rng);
    // Dopamine turns eligibility into weight (DOP-01): `dopamine` is its
    // integral over the interval that ends at `t` (level x ms).
    void reinforce(float dopamine, Millis t);
    // Adds `n` pyramidal cells, wired like the first ones, born at `now`.
    void grow(uint32_t n, Millis now, Rng& rng);

    // Spikes of each pyramidal cell during the last byte.
    const std::vector<uint8_t>& counts() const { return counts_; }
    uint32_t size() const { return uint32_t(pyramidal_.size()); }
    uint32_t interneurons() const { return uint32_t(interneurons_.size()); }
    uint32_t predicted() const;
    size_t synapses() const { return relay_.size(); }
    float mean_gain() const;
    const CortexParams& params() const { return p_; }

    template <class Archive>
    void persist(Archive& a) {
        uint32_t n = size();
        a(n);
        if constexpr (Archive::loading) resize(n);
        a(pyramidal_);
        a(homeostasis_);
        a(relay_);
        a(segments_);
        a(birth_);
        a(to_interneuron_start_);
        a(to_interneuron_);
        a(inhibitor_start_);
        a(inhibitor_);
        a(interneurons_);
        if constexpr (Archive::loading) index_inhibition();
        a(counts_);
        a(context_);
        a(predicted_);
    }

private:
    void wire(Millis now, Rng& rng);
    void resize(uint32_t n);
    void index_inhibition();
    float boost(uint32_t cell, Millis t) const;
    void tag(size_t index, float change, uint32_t cell, Millis t);

    CortexParams p_;
    uint32_t channels_;
    std::vector<Neuron> pyramidal_;
    std::vector<Homeostasis> homeostasis_;
    std::vector<Synapse> relay_;         // [cell * channels + channel]
    std::vector<Segment> segments_;      // [cell * segments + k]
    std::vector<double> birth_;          // ms
    std::vector<uint32_t> to_interneuron_start_, to_interneuron_;  // interneurons each cell excites, flattened
    std::vector<uint32_t> inhibitor_start_, inhibitor_;            // interneurons that inhibit each cell, flattened
    std::vector<std::vector<uint32_t>> inhibits_;                  // derived: cells each interneuron inhibits
    std::vector<Interneuron> interneurons_;
    std::vector<uint8_t> counts_, context_, predicted_;
    std::vector<uint32_t> fired_;        // scratch
};

}  // namespace child
