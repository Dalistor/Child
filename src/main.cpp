#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iterator>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

#include "brain/brain.hpp"
#include "brain/parameters.hpp"
#include "capture/capture.hpp"
#include "engram/engram.hpp"
#include "model/model.hpp"

using namespace child;

namespace {

const char* kUsage = R"(usage:
  child init   <ckpt> [--name N] [--seed S] [--engine brain|mlp] [--params FILE] [--neurons N]
                      (mlp only: [--embed E] [--context C] [--hidden H])
  child fork   <src> <dst> --name N
  child info   <ckpt>
  child feed   <ckpt> <path|-> [--epochs N] [--sleep C]   (mlp: [--lr F] [--batch N] [--seed S])
  child sleep  <ckpt> [--cycles N]
  child grow   <ckpt> --neurons N
  child sample <ckpt> [--prompt TEXT] [--len N] [--temp F] [--seed S]
  child params [<ckpt>]
  child bench  [--bytes N] [--neurons N]

environment:
  CHILD_ENGINE        brain (default) or mlp: the engine `init` creates
  CHILD_PARAMS        parameter file `init` reads when --params is absent
  CHILD_NEUROGENESIS  on (default) or off: whether a brain may create neurons while it learns
  CHILD_MAX_NEURONS   ceiling of pyramidal cells for this run (default: the base's neurogenesis.max_neurons)
)";

struct Args {
    std::vector<std::string> pos;
    std::map<std::string, std::string> opt;

    std::string str(const std::string& k, const std::string& def) const {
        auto it = opt.find(k);
        return it == opt.end() ? def : it->second;
    }
    double num(const std::string& k, double def) const {
        auto it = opt.find(k);
        return it == opt.end() ? def : std::stod(it->second);
    }
};

Args parse(int argc, char** argv) {
    Args a;
    for (int i = 2; i < argc; ++i) {
        std::string s = argv[i];
        if (s.rfind("--", 0) == 0) {
            if (i + 1 >= argc) throw std::runtime_error("missing value for " + s);
            a.opt[s.substr(2)] = argv[++i];
        } else {
            a.pos.push_back(s);
        }
    }
    return a;
}

void need(const Args& a, size_t n) {
    if (a.pos.size() != n) throw std::runtime_error(kUsage);
}

std::string env(const char* name, const std::string& def) {
    const char* v = std::getenv(name);
    return v && *v ? v : def;
}

std::string read_file(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    if (!in) throw std::runtime_error("cannot open " + path);
    return {std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>()};
}

// CHILD_NEUROGENESIS and CHILD_MAX_NEURONS steer growth for this run only.
void steer(Brain& brain) {
    const std::string growth = env("CHILD_NEUROGENESIS", "on");
    if (growth == "on" || growth == "1" || growth == "true") brain.neurogenesis().enabled = 1;
    else if (growth == "off" || growth == "0" || growth == "false") brain.neurogenesis().enabled = 0;
    else throw std::runtime_error("CHILD_NEUROGENESIS must be on or off, not '" + growth + "'");
    const std::string max = env("CHILD_MAX_NEURONS", "");
    if (max.empty()) return;
    char* end = nullptr;
    const unsigned long n = std::strtoul(max.c_str(), &end, 10);
    if (*end != '\0' || n == 0 || n > 1000000) throw std::runtime_error("CHILD_MAX_NEURONS must be an integer from 1 to 1000000, not '" + max + "'");
    brain.neurogenesis().max_neurons = uint32_t(n);
}

Brain open_brain(const Snapshot& s) {
    Brain brain = Brain::deserialize(s.brain);
    steer(brain);
    return brain;
}

std::string hex(uint64_t v) {
    char buf[17];
    std::snprintf(buf, sizeof buf, "%016llx", (unsigned long long)v);
    return buf;
}

void describe(const Snapshot& s) {
    std::printf("name     %s\n", s.lineage.name.c_str());
    std::printf("id       %s\n", hex(s.lineage.id).c_str());
    std::printf("parent   %s\n", s.lineage.parent ? hex(s.lineage.parent).c_str() : "-");
    std::printf("learned  %llu bytes\n", (unsigned long long)s.lineage.steps);
    if (!s.is_brain()) {
        std::printf("engine   mlp\n");
        std::printf("shape    embed=%u context=%u hidden=%u  (%zu parameters)\n", s.config.embed, s.config.context,
                    s.config.hidden, s.params.size());
        return;
    }
    const Brain b = Brain::deserialize(s.brain);
    std::printf("engine   brain\n");
    std::printf("cortex   %u pyramidal cells (%llu born), %u interneurons, %zu relay synapses\n", b.cortex().size(),
                (unsigned long long)b.born(), b.cortex().interneurons(), b.cortex().synapses());
    std::printf("cerebellum %u granule cells from %u mossy fibres\n", b.cerebellum().granules(), b.cerebellum().mossy());
    std::printf("hippocampus %u episodes of %llu seen\n", b.hippocampus().size(), (unsigned long long)b.hippocampus().seen());
    std::printf("time     %.1f s simulated\n", b.time() / 1000.0);
}

BrainParams brain_params(const Args& a) {
    BrainParams p;
    const std::string file = a.str("params", env("CHILD_PARAMS", ""));
    if (!file.empty()) configure(p, read_file(file));
    if (a.opt.count("neurons")) {
        const auto n = std::to_string(long(a.num("neurons", 0)));
        const auto inter = std::to_string(std::max(4L, long(a.num("neurons", 0)) / 4));
        configure(p, "cortex.excitatory = " + n + "\ncortex.interneurons = " + inter + "\n");
    }
    return p;
}

int cmd_init(const Args& a) {
    need(a, 1);
    const std::string engine = a.str("engine", env("CHILD_ENGINE", "brain"));
    const auto seed = uint64_t(a.num("seed", 1));
    Snapshot s;
    if (engine == "mlp") {
        Config cfg;
        cfg.embed = uint32_t(a.num("embed", cfg.embed));
        cfg.context = uint32_t(a.num("context", cfg.context));
        cfg.hidden = uint32_t(a.num("hidden", cfg.hidden));
        s = create(cfg, seed, a.str("name", "root"));
    } else if (engine == "brain") {
        s.lineage = {new_id(), 0, 0, a.str("name", "root")};
        s.brain = Brain(brain_params(a), seed).serialize();
    } else {
        throw std::runtime_error("unknown engine '" + engine + "' (brain or mlp)");
    }
    save(a.pos[0], s);
    describe(s);
    return 0;
}

int cmd_fork(const Args& a) {
    need(a, 2);
    if (!a.opt.count("name")) throw std::runtime_error("fork needs --name");
    Snapshot s = fork(load(a.pos[0]), a.opt.at("name"));
    save(a.pos[1], s);
    describe(s);
    return 0;
}

int cmd_info(const Args& a) {
    need(a, 1);
    describe(load(a.pos[0]));
    return 0;
}

void feed_mlp(const Args& a, Snapshot& s, const std::vector<uint8_t>& data, int epochs) {
    Model model(s.config, s.params);
    LearnOptions opts;
    opts.epochs = epochs;
    opts.lr = float(a.num("lr", opts.lr));
    opts.batch = size_t(a.num("batch", double(opts.batch)));
    opts.seed = uint64_t(a.num("seed", double(opts.seed)));
    learn(model, data, opts, [](int epoch, float loss) { std::printf("epoch %3d  loss %.4f\n", epoch, loss); });
    s.params = model.params();
}

void feed_brain(const Args& a, Snapshot& s, const std::vector<uint8_t>& data, int epochs) {
    Brain brain = open_brain(s);
    const auto cycles = uint32_t(a.num("sleep", 0));
    for (int epoch = 1; epoch <= epochs; ++epoch) {
        const uint32_t before = brain.cortex().size();
        double surprise = 0;
        for (uint8_t byte : data) surprise += brain.perceive(byte, true);
        if (cycles) brain.sleep(cycles);
        std::printf("epoch %3d  loss %.4f  neurons %u", epoch, surprise / double(data.size()), brain.cortex().size());
        if (brain.cortex().size() > before) std::printf(" (+%u born)", brain.cortex().size() - before);
        std::printf("\n");
    }
    s.brain = brain.serialize();
}

int cmd_feed(const Args& a) {
    need(a, 2);
    Snapshot s = load(a.pos[0]);
    std::vector<uint8_t> data = capture(a.pos[1]);
    if (data.empty()) throw std::runtime_error("captured nothing from " + a.pos[1]);
    std::printf("captured %zu bytes\n", data.size());
    const int epochs = int(a.num("epochs", 1));
    if (epochs < 1) throw std::runtime_error("--epochs must be at least 1");
    if (s.is_brain()) feed_brain(a, s, data, epochs);
    else feed_mlp(a, s, data, epochs);
    s.lineage.steps += uint64_t(data.size()) * uint64_t(epochs);
    save(a.pos[0], s);
    return 0;
}

int cmd_sleep(const Args& a) {
    need(a, 1);
    Snapshot s = load(a.pos[0]);
    if (!s.is_brain()) throw std::runtime_error("sleep needs a brain base; the mlp has no hippocampus");
    Brain brain = open_brain(s);
    const auto cycles = uint32_t(a.num("cycles", 1));
    brain.sleep(cycles);
    std::printf("slept %u cycle(s), replaying %u episodes\n", cycles, brain.hippocampus().size());
    s.brain = brain.serialize();
    save(a.pos[0], s);
    return 0;
}

int cmd_grow(const Args& a) {
    need(a, 1);
    Snapshot s = load(a.pos[0]);
    if (!s.is_brain()) throw std::runtime_error("grow needs a brain base");
    Brain brain = open_brain(s);
    const auto wanted = uint32_t(a.num("neurons", 0));
    const uint32_t room = brain.neurogenesis().max_neurons > brain.cortex().size()
                              ? brain.neurogenesis().max_neurons - brain.cortex().size()
                              : 0;
    if (wanted == 0) throw std::runtime_error("grow needs --neurons N above 0");
    const uint32_t added = brain.grow(std::min(wanted, room));
    std::printf("grew %u pyramidal cell(s), now %u (ceiling %u)\n", added, brain.cortex().size(),
                brain.neurogenesis().max_neurons);
    s.brain = brain.serialize();
    save(a.pos[0], s);
    return 0;
}

uint8_t draw(const float* probs, float temp, float u) {
    std::vector<double> w(kOutputs);
    double sum = 0;
    for (uint32_t v = 0; v < kOutputs; ++v) sum += (w[v] = std::pow(double(probs[v]), 1.0 / temp));
    double acc = 0, r = double(u) * sum;
    for (uint32_t v = 0; v < kOutputs; ++v)
        if (r < (acc += w[v])) return uint8_t(v);
    return uint8_t(kOutputs - 1);
}

int cmd_sample(const Args& a) {
    need(a, 1);
    Snapshot s = load(a.pos[0]);
    std::string prompt = a.str("prompt", "");
    std::vector<uint8_t> text(prompt.begin(), prompt.end());
    size_t len = size_t(a.num("len", 200));
    float temp = float(a.num("temp", 0.8));
    if (temp <= 0.f) throw std::runtime_error("--temp must be positive");
    Rng rng(uint64_t(a.num("seed", 1)));

    std::fwrite(prompt.data(), 1, prompt.size(), stdout);
    if (s.is_brain()) {
        Brain brain = open_brain(s);  // perceives without learning; the base on disk is untouched
        for (uint8_t byte : text) brain.perceive(byte, false);
        for (size_t n = 0; n < len; ++n) {
            const uint8_t next = draw(brain.prediction(), temp, rng.uniform());
            std::fputc(next, stdout);
            brain.perceive(next, false);
        }
    } else {
        Model model(s.config, s.params);
        std::vector<uint8_t> ctx(s.config.context);
        std::vector<float> probs(kVocab);
        for (size_t n = 0; n < len; ++n) {
            window(text, text.size(), s.config.context, ctx.data());
            model.predict(ctx.data(), probs.data());
            const uint8_t next = draw(probs.data(), temp, rng.uniform());
            text.push_back(next);
            std::fputc(next, stdout);
        }
    }
    std::fputc('\n', stdout);
    return 0;
}

int cmd_params(const Args& a) {
    if (a.pos.size() > 1) throw std::runtime_error(kUsage);
    BrainParams p;
    if (a.pos.size() == 1) {
        Snapshot s = load(a.pos[0]);
        if (!s.is_brain()) throw std::runtime_error("params needs a brain base");
        p = Brain::deserialize(s.brain).params();
    }
    std::printf("%-36s %12s %12s %12s  %-8s %s\n", "name", "value", "min", "max", "unit", "source");
    for (const auto& q : parameters(p))
        std::printf("%-36s %12.6g %12.6g %12.6g  %-8s %s\n", q.name.c_str(), q.get(), q.min, q.max, q.unit.c_str(),
                    q.source.c_str());
    return 0;
}

int cmd_bench(const Args& a) {
    if (!a.pos.empty()) throw std::runtime_error(kUsage);
    const BrainParams p = brain_params(a);
    Brain brain(p, 1);
    steer(brain);
    brain.neurogenesis().enabled = 0;  // fixed size, so the measure is of one network
    const auto bytes = size_t(a.num("bytes", 2000));
    Rng rng(7);
    const auto start = std::chrono::steady_clock::now();
    for (size_t i = 0; i < bytes; ++i) brain.perceive(uint8_t(' ' + rng.below(95)), true);
    const double seconds = std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
    const double cells = double(brain.cortex().size() + brain.cortex().interneurons());
    const double steps = double(bytes) * double(std::lround(p.byte_ms / p.step_ms));
    std::printf("cells    %.0f (%u pyramidal, %u interneurons), %zu relay synapses, %u granule cells\n", cells,
                brain.cortex().size(), brain.cortex().interneurons(), brain.cortex().synapses(), brain.cerebellum().granules());
    std::printf("bytes/s  %.0f (%.1f s simulated per s)\n", double(bytes) / seconds, double(bytes) * p.byte_ms / 1000.0 / seconds);
    std::printf("neuron-steps/s  %.3g (step %.2f ms, one CPU thread)\n", cells * steps / seconds, double(p.step_ms));
    return 0;
}

}  // namespace

int main(int argc, char** argv) {
    if (argc < 2) {
        std::fputs(kUsage, stderr);
        return 2;
    }
    try {
        std::string cmd = argv[1];
        Args a = parse(argc, argv);
        if (cmd == "init") return cmd_init(a);
        if (cmd == "fork") return cmd_fork(a);
        if (cmd == "info") return cmd_info(a);
        if (cmd == "feed") return cmd_feed(a);
        if (cmd == "sleep") return cmd_sleep(a);
        if (cmd == "grow") return cmd_grow(a);
        if (cmd == "sample") return cmd_sample(a);
        if (cmd == "params") return cmd_params(a);
        if (cmd == "bench") return cmd_bench(a);
        std::fputs(kUsage, stderr);
        return 2;
    } catch (const std::exception& e) {
        std::fprintf(stderr, "error: %s\n", e.what());
        return 1;
    }
}
