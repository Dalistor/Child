#pragma once

#include <cstdint>
#include <vector>

#include "model/rng.hpp"

namespace child {

// Dendrites make a neuron more than a point (docs/pesquisa-degraus.md).
// Two kinds, both reduced to what they do for the soma: a current.

// Calcium action potential of human layer 2/3 apical dendrites (Gidon et al.
// 2020). Graded and non-monotonic: nothing below threshold, maximal at
// threshold, damped for stronger input. That lets one neuron compute XOR.
struct CalciumParams {
    float threshold = 1.f;      // drive that triggers it, in the caller's units
    float decay = 0.39f;        // amplitude decay above threshold, in units of threshold (Gidon: 0.39)
    float peak_current = 600.f; // pA delivered to the soma at full amplitude
};

// Amplitude in [0, 1] for a given drive.
float calcium_amplitude(float drive, const CalciumParams& params);

// A distal segment whose coincident inputs trigger an NMDA plateau (Schiller
// et al. 2000; Antic et al. 2010). As in Hawkins & Ahmad 2016, each synapse has
// a permanence, counts once it passes `connected`, and the plateau depolarizes
// the soma: the neuron is "predicted" by the context those inputs carry.
struct SegmentParams {
    uint32_t synapses = 16;         // per segment (Hawkins & Ahmad: 20-40)
    uint32_t threshold = 4;         // coincident active synapses (8-20 in cortex; fewer for a small network)
    float connected = 0.5f;         // permanence from which a synapse counts
    float initial = 0.45f;          // permanence of a new synapse
    float increment = 0.1f;         // on active synapses when the neuron fires (< 0.1)
    float decrement = 0.02f;        // on inactive ones
    float plateau_current = 150.f;  // pA while the plateau lasts
};

class Segment {
public:
    Segment() = default;
    Segment(uint32_t synapses, uint32_t population, float initial, Rng& rng);

    // Connected synapses whose presynaptic cell is active (`active` is indexed by cell).
    uint32_t overlap(const std::vector<uint8_t>& active, float connected) const;
    // Active synapses regardless of permanence: how close the segment is to the context.
    uint32_t potential(const std::vector<uint8_t>& active) const;

    // Local Hebbian rule after the neuron fired: active synapses grow, inactive
    // ones shrink, and the weakest unconnected synapse moves to an active cell
    // it does not reach yet (structural plasticity).
    void learn(const std::vector<uint8_t>& active, const SegmentParams& params, Rng& rng);

    template <class Archive>
    void persist(Archive& a) {
        a(pre_);
        a(permanence_);
    }

private:
    std::vector<uint32_t> pre_;
    std::vector<float> permanence_;
};

}  // namespace child
