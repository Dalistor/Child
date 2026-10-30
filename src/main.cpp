#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

#include "capture.hpp"
#include "model.hpp"
#include "store.hpp"

using namespace child;

namespace {

const char* kUsage = R"(usage:
  child init   <ckpt> [--name N] [--seed S] [--embed E] [--context C] [--hidden H]
  child fork   <src> <dst> --name N
  child info   <ckpt>
  child feed   <ckpt> <path|-> [--epochs N] [--lr F] [--batch N] [--seed S]
  child sample <ckpt> [--prompt TEXT] [--len N] [--temp F] [--seed S]
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
    std::printf("shape    embed=%u context=%u hidden=%u  (%zu parameters)\n", s.config.embed, s.config.context,
                s.config.hidden, s.params.size());
}

int cmd_init(const Args& a) {
    need(a, 1);
    Config cfg;
    cfg.embed = uint32_t(a.num("embed", cfg.embed));
    cfg.context = uint32_t(a.num("context", cfg.context));
    cfg.hidden = uint32_t(a.num("hidden", cfg.hidden));
    Snapshot s = create(cfg, uint64_t(a.num("seed", 1)), a.str("name", "root"));
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

int cmd_feed(const Args& a) {
    need(a, 2);
    Snapshot s = load(a.pos[0]);
    std::vector<uint8_t> data = capture(a.pos[1]);
    if (data.empty()) throw std::runtime_error("captured nothing from " + a.pos[1]);
    std::printf("captured %zu bytes\n", data.size());

    Model model(s.config, s.params);
    LearnOptions opts;
    opts.epochs = int(a.num("epochs", opts.epochs));
    opts.lr = float(a.num("lr", opts.lr));
    opts.batch = size_t(a.num("batch", double(opts.batch)));
    opts.seed = uint64_t(a.num("seed", double(opts.seed)));

    learn(model, data, opts, [](int epoch, float loss) { std::printf("epoch %3d  loss %.4f\n", epoch, loss); });

    s.params = model.params();
    s.lineage.steps += uint64_t(data.size()) * uint64_t(opts.epochs);
    save(a.pos[0], s);
    return 0;
}

int cmd_sample(const Args& a) {
    need(a, 1);
    Snapshot s = load(a.pos[0]);
    Model model(s.config, s.params);

    std::string prompt = a.str("prompt", "");
    std::vector<uint8_t> text(prompt.begin(), prompt.end());
    size_t len = size_t(a.num("len", 200));
    float temp = float(a.num("temp", 0.8));
    if (temp <= 0.f) throw std::runtime_error("--temp must be positive");

    uint64_t state = uint64_t(a.num("seed", 1));
    auto uniform = [&state] {
        state = state * 6364136223846793005ULL + 1442695040888963407ULL;
        return float(state >> 40) / float(1ULL << 24);
    };

    std::vector<uint8_t> ctx(s.config.context);
    std::vector<float> probs(kVocab);
    std::fwrite(prompt.data(), 1, prompt.size(), stdout);
    for (size_t n = 0; n < len; ++n) {
        window(text, text.size(), s.config.context, ctx.data());
        model.predict(ctx.data(), probs.data());

        double sum = 0;
        for (float& p : probs) sum += (p = std::pow(p, 1.f / temp));
        float r = uniform() * float(sum), acc = 0;
        uint8_t next = kVocab - 1;
        for (uint32_t v = 0; v < kVocab; ++v) {
            acc += probs[v];
            if (r < acc) {
                next = uint8_t(v);
                break;
            }
        }
        text.push_back(next);
        std::fputc(next, stdout);
    }
    std::fputc('\n', stdout);
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
        if (cmd == "sample") return cmd_sample(a);
        std::fputs(kUsage, stderr);
        return 2;
    } catch (const std::exception& e) {
        std::fprintf(stderr, "error: %s\n", e.what());
        return 1;
    }
}
