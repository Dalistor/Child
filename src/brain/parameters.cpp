#include "parameters.hpp"

#include <charconv>
#include <cmath>
#include <cstdio>
#include <sstream>
#include <stdexcept>

namespace child {

bool Parameter::admits(double v) const {
    if (real) return float(v) >= float(min) && float(v) <= float(max);
    return v >= min && v <= max;
}

void Parameter::set(double v) const {
    if (real) *real = float(v);
    else *integer = uint32_t(v);
}

std::vector<Parameter> parameters(BrainParams& b) {
    auto& c = b.cortex;
    auto& n = c.neuron;
    auto& s = c.synapse;
    auto& d = c.dendrite;
    auto& g = b.neurogenesis;
    auto r = [](std::string name, std::string unit, double lo, double hi, std::string src, float& f) {
        return Parameter{std::move(name), std::move(unit), lo, hi, std::move(src), &f, nullptr};
    };
    auto i = [](std::string name, std::string unit, double lo, double hi, std::string src, uint32_t& u) {
        return Parameter{std::move(name), std::move(unit), lo, hi, std::move(src), nullptr, &u};
    };
    const std::string eng = "engenharia";
    return {
        r("brain.byte_ms", "ms", 5, 50, eng, b.byte_ms),
        r("brain.step_ms", "ms", 0.05, 0.1, "BG05 (Euler)", b.step_ms),
        r("brain.context_tau", "ms", 10, 100, eng, b.context_tau),
        r("brain.dopamine_tau", "ms", 100, 500, "Izhikevich 2007", b.dopamine_tau),
        r("brain.dopamine_gain", "-", 0, 10, eng, b.dopamine_gain),
        r("brain.reinforce_ms", "ms", 10, 200, "abaixo de tau_d e tau_e (Izhikevich 2007)", b.reinforce_ms),
        r("brain.sensory_weight", "-", 0, 1, eng, b.sensory_weight),
        r("brain.expectation_bytes", "bytes", 1, 10000, eng, b.expectation_bytes),

        i("thalamus.channels", "-", 64, 4096, eng, b.thalamus.channels),
        i("thalamus.channels_per_byte", "-", 4, 64, eng, b.thalamus.channels_per_byte),
        r("thalamus.active_rate", "Hz", 50, 300, eng, b.thalamus.active_rate),
        r("thalamus.spontaneous_rate", "Hz", 0, 10, eng, b.thalamus.spontaneous_rate),

        r("neuron.capacitance", "pF", 59, 281, "BG05; Naud 2008", n.capacitance),
        r("neuron.leak", "nS", 1.7, 30, "BG05; Naud 2008", n.leak),
        r("neuron.rest", "mV", -70.6, -58, "BG05; Naud 2008", n.rest),
        r("neuron.threshold", "mV", -56, -42, "BG05; Naud 2008", n.threshold),
        r("neuron.slope", "mV", 0.8, 5.5, "BG05; Naud 2008", n.slope),
        r("neuron.subthreshold", "nS", -11, 4, "BG05; Naud 2008", n.subthreshold),
        r("neuron.adaptation_tau", "ms", 16, 300, "BG05; Naud 2008", n.adaptation_tau),
        r("neuron.spike_jump", "pA", 0, 120, "BG05; Naud 2008", n.spike_jump),
        r("neuron.reset", "mV", -70.6, -46, "BG05; Naud 2008", n.reset),
        r("neuron.peak", "mV", 0, 20, "BG05; Naud 2008", n.peak),

        r("synapse.potentiation", "-", 0.005, 0.03125, "Song 2000; Masquelier 2008", s.potentiation),
        r("synapse.depression_ratio", "-", 0.85, 1.05, "Song 2000; Masquelier 2008", s.depression_ratio),
        r("synapse.tau_plus", "ms", 16.8, 20, "Song 2000; Masquelier 2008", s.tau_plus),
        r("synapse.tau_minus", "ms", 20, 33.7, "Song 2000; Masquelier 2008", s.tau_minus),
        r("eligibility.tau", "ms", 100, 1000, "Fremaux & Gerstner 2016; Izhikevich 2007", c.eligibility.tau),
        r("eligibility.lifetime", "ms", 1000, 25000, "Izhikevich 2007", c.eligibility.lifetime),
        r("homeostasis.target_rate", "Hz", 3, 6, "Zenke 2013; Vogels 2011", c.homeostasis.target_rate),
        r("homeostasis.tau", "ms", 1000, 20000, "cerebro@2 HOM-01; Zenke 2013", c.homeostasis.tau),
        r("homeostasis.learning_rate", "1/ms", 1e-5, 1e-3, eng, c.homeostasis.learning_rate),
        r("interneuron.conductance", "nS", 0.3, 100, "Vogels 2011", c.interneuron.conductance),

        i("cortex.excitatory", "-", 16, 65536, eng, c.excitatory),
        i("cortex.interneurons", "-", 4, 16384, "4:1 (Vogels 2011; Izhikevich 2007)", c.interneurons),
        r("cortex.thalamic_peak", "nS", 0.45, 5, "Song 2000 (0,45-1,05) e acima para redes pequenas", c.thalamic_peak),
        r("cortex.initial_weight", "-", 0, 1, eng, c.initial_weight),
        r("cortex.initial_spread", "-", 0, 0.5, eng, c.initial_spread),
        i("cortex.to_interneuron_inputs", "-", 1, 10000, "Vogels 2011 (2% de 8000 = 160)", c.to_interneuron_inputs),
        i("cortex.from_interneuron_inputs", "-", 1, 10000, "Vogels 2011 (2% de 2000 = 40); celulas em cesto", c.from_interneuron_inputs),
        r("cortex.to_interneuron_conductance", "nS", 0.3, 3, "Vogels 2011", c.to_interneuron_conductance),
        i("cortex.segments", "-", 0, 16, "Hawkins & Ahmad 2016 (ate 128)", c.segments),
        r("cortex.young_boost", "-", 1, 4, "Ge 2007 (LTP aumentada)", c.young_boost),
        r("cortex.young_ms", "ms", 0, 1e7, eng, c.young_ms),

        i("dendrite.synapses", "-", 4, 40, "Hawkins & Ahmad 2016 (20-40)", d.synapses),
        i("dendrite.threshold", "-", 2, 20, "Hawkins & Ahmad 2016 (8-20)", d.threshold),
        r("dendrite.connected", "-", 0, 1, "Hawkins & Ahmad 2016 (0,5)", d.connected),
        r("dendrite.initial", "-", 0, 1, eng, d.initial),
        r("dendrite.increment", "-", 0, 0.1, "Hawkins & Ahmad 2016 (< 0,1)", d.increment),
        r("dendrite.decrement", "-", 0, 0.1, "Hawkins & Ahmad 2016 (< 0,1)", d.decrement),
        r("dendrite.plateau_current", "pA", 0, 500, "Schiller 2000 (5,9 mV no soma)", d.plateau_current),

        i("cerebellum.expansion", "-", 1, 50, "Litwin-Kumar 2017 (10-50)", b.cerebellum.expansion),
        i("cerebellum.inputs", "-", 2, 7, "Billings 2014 (2-7)", b.cerebellum.inputs),
        r("cerebellum.coding_level", "-", 0.01, 0.1, "Litwin-Kumar 2017 (0,01-0,1)", b.cerebellum.coding_level),
        r("cerebellum.learning_rate", "-", 0.01, 2, eng, b.cerebellum.learning_rate),

        i("hippocampus.capacity", "episodes", 0, 1000000, eng, b.hippocampus.capacity),
        i("hippocampus.replay", "-", 1, 20, eng, b.hippocampus.replay),

        i("neurogenesis.enabled", "-", 0, 1, eng, g.enabled),
        i("neurogenesis.max_neurons", "-", 16, 1000000, eng, g.max_neurons),
        i("neurogenesis.window", "bytes", 64, 1000000, "Draelos 2017; Fritzke 1995", g.window),
        r("neurogenesis.min_surprise", "nats", 0, 5.55, eng, g.min_surprise),
        r("neurogenesis.min_improvement", "-", 0, 1, eng, g.min_improvement),
        i("neurogenesis.batch", "-", 1, 4096, eng, g.batch),
    };
}

namespace {

std::string trim(const std::string& s) {
    const auto a = s.find_first_not_of(" \t\r");
    if (a == std::string::npos) return "";
    const auto b = s.find_last_not_of(" \t\r");
    return s.substr(a, b - a + 1);
}

std::string format(double v) {
    char buf[32];
    std::snprintf(buf, sizeof buf, "%.9g", v);
    return buf;
}

}  // namespace

void configure(BrainParams& params, const std::string& text) {
    const auto registry = parameters(params);
    std::istringstream in(text);
    std::string line;
    for (int number = 1; std::getline(in, line); ++number) {
        line = trim(line.substr(0, line.find('#')));
        if (line.empty()) continue;
        const auto eq = line.find('=');
        if (eq == std::string::npos) throw std::runtime_error("parameters line " + std::to_string(number) + ": expected name = value");
        const std::string name = trim(line.substr(0, eq)), value = trim(line.substr(eq + 1));
        const Parameter* p = nullptr;
        for (const auto& candidate : registry)
            if (candidate.name == name) p = &candidate;
        if (!p) throw std::runtime_error("unknown parameter " + name);
        double v = 0;
        const auto [end, err] = std::from_chars(value.data(), value.data() + value.size(), v);
        if (err != std::errc() || end != value.data() + value.size() || !std::isfinite(v))
            throw std::runtime_error("parameter " + name + ": '" + value + "' is not a number");
        if (p->integer && v != std::floor(v)) throw std::runtime_error("parameter " + name + " must be an integer");
        // 0.1 written with 9 digits reads back as 0.100000001: admits() compares in float32.
        if (!p->admits(v))
            throw std::runtime_error("parameter " + name + " = " + value + " is outside its range [" + format(p->min) +
                                     ", " + format(p->max) + "] " + p->unit);
        p->set(v);
    }
}

std::string describe(const BrainParams& params) {
    BrainParams copy = params;
    std::string out;
    for (const auto& p : parameters(copy)) out += p.name + " = " + format(p.get()) + "\n";
    return out;
}

}  // namespace child
