#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "cerebellum/cerebellum.hpp"
#include "cortex/cortex.hpp"
#include "hippocampus/hippocampus.hpp"
#include "brain/neurogenesis.hpp"
#include "thalamus/thalamus.hpp"

namespace child {

// Everything a base is built from. Defaults and ranges come from
// docs/parametros.md (`child params` prints this registry).
struct BrainParams {
    float byte_ms = 10.f;            // how long each byte is presented
    float step_ms = 0.1f;            // integration step
    float context_tau = 30.f;        // ms, trace of cortical spikes the cerebellum reads
    float dopamine_tau = 200.f;      // ms
    float dopamine_gain = 1.f;       // release per nat of surprise above the expected surprise
    float reinforce_ms = 50.f;
    float sensory_weight = 1.f;       // weight of the direct relay input to the cerebellum against the cortical one       // dopamine reaches the synapses in steps of this length
    float expectation_bytes = 100.f; // bytes over which the expected surprise is averaged

    ThalamusParams thalamus;
    CortexParams cortex;
    CerebellumParams cerebellum;
    HippocampusParams hippocampus;
    NeurogenesisParams neurogenesis;
};

// One named, ranged parameter, bound to a field of a BrainParams.
struct Parameter {
    std::string name;
    std::string unit;
    double min, max;
    std::string source;
    float* real = nullptr;        // one of the two is set
    uint32_t* integer = nullptr;

    double get() const { return real ? double(*real) : double(*integer); }
    void set(double v) const;
    // In range at the parameter's own precision (float32 for real ones).
    bool admits(double v) const;
};

std::vector<Parameter> parameters(BrainParams& params);

// "name = value" lines, '#' starts a comment. Missing names keep their
// default; an unknown name, a malformed value or one outside its range
// throws, naming the parameter.
void configure(BrainParams& params, const std::string& text);

// Every parameter as "name = value" lines, in registry order: reading it back
// with configure() gives the same parameters.
std::string describe(const BrainParams& params);

}  // namespace child
