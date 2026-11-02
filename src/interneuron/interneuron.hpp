#pragma once

#include "neuron/neuron.hpp"

namespace child {

// The generic inhibitory neuron of cerebro@2 (PV, SST and VIP types come
// later). Its cell is a fast AdEx without spike-triggered adaptation: the
// tonic set of Naud et al. 2008. Ranges and sources are in docs/parametros.md.
AdExParams fast_spiking();

struct InterneuronParams {
    AdExParams cell = fast_spiking();
    float conductance = 3.f;  // nS opened in each target per spike (Vogels et al. 2011 range)
};

// Collects excitation from the population and, when it fires, inhibits its
// targets through a fixed synapse: it does not learn by the excitatory STDP
// rule in this version.
class Interneuron {
public:
    explicit Interneuron(const InterneuronParams& params = {});

    void excite(float conductance) { cell_.excite(conductance); }
    bool step(float current, float dt = kStepMs) { return cell_.step(current, dt); }
    void inhibit(Neuron& target) const { target.inhibit(p_.conductance); }

    float conductance() const { return p_.conductance; }
    float potential() const { return cell_.potential(); }
    const InterneuronParams& params() const { return p_; }

private:
    InterneuronParams p_;
    Neuron cell_;
};

}  // namespace child
