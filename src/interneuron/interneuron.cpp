#include "interneuron.hpp"

namespace child {

AdExParams fast_spiking() {
    AdExParams p;
    p.capacitance = 200.f;
    p.leak = 10.f;
    p.rest = -70.f;
    p.threshold = -50.f;
    p.slope = 2.f;
    p.subthreshold = 2.f;
    p.adaptation_tau = 30.f;
    p.spike_jump = 0.f;
    p.reset = -58.f;
    return p;
}

Interneuron::Interneuron(const InterneuronParams& params) : p_(params), cell_(params.cell) {}

}  // namespace child
