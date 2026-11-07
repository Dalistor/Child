#pragma once

#include <cstdint>
#include <vector>

#include "brain/neurogenesis.hpp"
#include "brain/parameters.hpp"
#include "cerebellum/cerebellum.hpp"
#include "cortex/cortex.hpp"
#include "dopamine/dopamine.hpp"
#include "hippocampus/hippocampus.hpp"
#include "model/rng.hpp"
#include "thalamus/thalamus.hpp"

namespace child {

// The simplified brain of docs/sintese.md, wired:
//
//   byte -> thalamus -> cortex (spiking, STDP x dopamine, dendrites, inhibition)
//                         |  trace of recent spikes
//                         v
//                     cerebellum -> P(next byte) ------> surprise -> dopamine
//                         ^                                   |
//                         | replay during sleep               v
//                     hippocampus (episodes)          neurogenesis (more cells)
//
// Each byte is presented for params.byte_ms of simulated time. Everything is
// deterministic from the seed, and the whole dynamic state can be saved and
// restored (engram section "brain").
class Brain {
public:
    Brain(const BrainParams& params, uint64_t seed);

    // Presents `byte`. Returns its surprise, -ln P(byte), under the prediction
    // made after the previous byte (ln 256 for the first). With `learn`, the
    // cerebellum, the hippocampus, dopamine, the cortex and neurogenesis all
    // act on it; without, the brain only perceives (weights stay as they are).
    float perceive(uint8_t byte, bool learn);

    // P(next byte), kOutputs floats, after the last perceive().
    const float* prediction() const { return probs_.data(); }

    // Replays the hippocampal episodes to the cerebellum, `cycles` times.
    void sleep(uint32_t cycles);

    // Adds `n` pyramidal cells (and the granule cells that read them). Returns how many were added.
    uint32_t grow(uint32_t n);

    // Runtime control of growth (CHILD_NEUROGENESIS / CHILD_MAX_NEURONS); not saved.
    NeurogenesisParams& neurogenesis() { return neurogenesis_.params(); }

    const BrainParams& params() const { return p_; }
    uint64_t seed() const { return seed_; }
    const Cortex& cortex() const { return cortex_; }
    const Cerebellum& cerebellum() const { return cerebellum_; }
    const Hippocampus& hippocampus() const { return hippocampus_; }
    uint64_t born() const { return neurogenesis_.born(); }
    double time() const { return time_; }       // ms simulated
    float dopamine() const { return released_; }  // released at the last perceive()
    float expected_surprise() const { return expected_; }

    // Engram section "brain": parameters as text, then the whole state.
    std::vector<uint8_t> serialize() const;
    static Brain deserialize(const std::vector<uint8_t>& bytes);

private:
    template <class Archive>
    void persist(Archive& a);
    void read_out();

    BrainParams p_;
    uint64_t seed_;
    Rng rng_;
    Thalamus thalamus_;
    Cortex cortex_;
    Cerebellum cerebellum_;
    Hippocampus hippocampus_;
    Dopamine dopamine_;
    Neurogenesis neurogenesis_;

    double time_ = 0;
    float expected_;
    float released_ = 0.f;
    float dopamine_integral_ = 0.f;  // since the last reinforcement, level x ms
    uint32_t since_reinforce_ = 0;   // bytes
    uint8_t has_prediction_ = 0;
    std::vector<float> trace_;       // what the cerebellum reads: relay channels, then cortical cells
    std::vector<uint16_t> relayed_;  // relay spikes per channel during the last byte
    std::vector<uint32_t> active_;   // granule cells behind the current prediction
    std::vector<float> probs_;
    std::vector<uint32_t> relay_;    // scratch
};

}  // namespace child
