#include "model.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>

#include "rng.hpp"

namespace child {

size_t Config::param_count() const {
    size_t in = size_t(context) * embed;
    return size_t(kVocab) * embed + size_t(hidden) * in + hidden + size_t(kVocab) * hidden + kVocab;
}

Model::Model(const Config& cfg, uint64_t seed) : cfg_(cfg), p_(cfg.param_count()) {
    layout();
    Rng rng(seed);
    auto fill = [&](size_t off, size_t n, float limit) {
        for (size_t i = 0; i < n; ++i) p_[off + i] = (rng.uniform() * 2.f - 1.f) * limit;
    };
    size_t in = size_t(cfg_.context) * cfg_.embed;
    fill(o_emb_, size_t(kVocab) * cfg_.embed, 0.1f);
    fill(o_w1_, size_t(cfg_.hidden) * in, std::sqrt(6.f / float(in + cfg_.hidden)));
    fill(o_w2_, size_t(kVocab) * cfg_.hidden, std::sqrt(6.f / float(cfg_.hidden + kVocab)));
}

Model::Model(const Config& cfg, std::vector<float> params) : cfg_(cfg), p_(std::move(params)) {
    if (p_.size() != cfg_.param_count()) throw std::runtime_error("parameter count does not match config");
    layout();
}

void Model::layout() {
    size_t in = size_t(cfg_.context) * cfg_.embed;
    o_emb_ = 0;
    o_w1_ = o_emb_ + size_t(kVocab) * cfg_.embed;
    o_b1_ = o_w1_ + size_t(cfg_.hidden) * in;
    o_w2_ = o_b1_ + cfg_.hidden;
    o_b2_ = o_w2_ + size_t(kVocab) * cfg_.hidden;

    g_.assign(p_.size(), 0.f);
    x_.assign(in, 0.f);
    dx_.assign(in, 0.f);
    h_.assign(cfg_.hidden, 0.f);
    dh_.assign(cfg_.hidden, 0.f);
    probs_.assign(kVocab, 0.f);
}

void Model::forward(const uint8_t* ctx) {
    const size_t E = cfg_.embed, C = cfg_.context, H = cfg_.hidden, in = C * E;

    for (size_t j = 0; j < C; ++j) {
        const float* row = &p_[o_emb_ + size_t(ctx[j]) * E];
        std::copy(row, row + E, &x_[j * E]);
    }

    for (size_t r = 0; r < H; ++r) {
        const float* w = &p_[o_w1_ + r * in];
        float s = p_[o_b1_ + r];
        for (size_t k = 0; k < in; ++k) s += w[k] * x_[k];
        h_[r] = std::tanh(s);
    }

    float top = -INFINITY;
    for (size_t v = 0; v < kVocab; ++v) {
        const float* w = &p_[o_w2_ + v * H];
        float s = p_[o_b2_ + v];
        for (size_t k = 0; k < H; ++k) s += w[k] * h_[k];
        probs_[v] = s;
        top = std::max(top, s);
    }

    float sum = 0.f;
    for (size_t v = 0; v < kVocab; ++v) sum += (probs_[v] = std::exp(probs_[v] - top));
    for (size_t v = 0; v < kVocab; ++v) probs_[v] /= sum;
}

void Model::predict(const uint8_t* ctx, float* probs) {
    forward(ctx);
    std::copy(probs_.begin(), probs_.end(), probs);
}

float Model::backward(const uint8_t* ctx, uint8_t target) {
    const size_t E = cfg_.embed, C = cfg_.context, H = cfg_.hidden, in = C * E;

    forward(ctx);
    float loss = -std::log(std::max(probs_[target], 1e-12f));

    // probs_ becomes d(loss)/d(logits)
    probs_[target] -= 1.f;

    std::fill(dh_.begin(), dh_.end(), 0.f);
    for (size_t v = 0; v < kVocab; ++v) {
        float d = probs_[v];
        const float* w = &p_[o_w2_ + v * H];
        float* gw = &g_[o_w2_ + v * H];
        g_[o_b2_ + v] += d;
        for (size_t k = 0; k < H; ++k) {
            gw[k] += d * h_[k];
            dh_[k] += d * w[k];
        }
    }

    std::fill(dx_.begin(), dx_.end(), 0.f);
    for (size_t r = 0; r < H; ++r) {
        float dz = dh_[r] * (1.f - h_[r] * h_[r]);
        const float* w = &p_[o_w1_ + r * in];
        float* gw = &g_[o_w1_ + r * in];
        g_[o_b1_ + r] += dz;
        for (size_t k = 0; k < in; ++k) {
            gw[k] += dz * x_[k];
            dx_[k] += dz * w[k];
        }
    }

    for (size_t j = 0; j < C; ++j) {
        float* grow = &g_[o_emb_ + size_t(ctx[j]) * E];
        for (size_t e = 0; e < E; ++e) grow[e] += dx_[j * E + e];
    }

    return loss;
}

void Model::zero_grad() { std::fill(g_.begin(), g_.end(), 0.f); }

void Model::step(float lr) {
    for (size_t i = 0; i < p_.size(); ++i) p_[i] -= lr * g_[i];
}

void window(const std::vector<uint8_t>& data, size_t pos, uint32_t context, uint8_t* ctx) {
    for (uint32_t j = 0; j < context; ++j) {
        size_t back = context - j;  // distance from pos
        ctx[j] = pos >= back ? data[pos - back] : 0;
    }
}

float learn(Model& model, const std::vector<uint8_t>& data, const LearnOptions& opts,
            const std::function<void(int, float)>& on_epoch) {
    if (data.empty()) throw std::runtime_error("nothing to learn from");

    std::vector<uint32_t> order(data.size());
    for (size_t i = 0; i < order.size(); ++i) order[i] = uint32_t(i);

    Rng rng(opts.seed);
    std::vector<uint8_t> ctx(model.config().context);
    float mean = 0.f;

    for (int epoch = 1; epoch <= opts.epochs; ++epoch) {
        for (size_t i = order.size(); i > 1; --i) std::swap(order[i - 1], order[rng.next() % i]);

        double total = 0.0;
        for (size_t start = 0; start < order.size(); start += opts.batch) {
            size_t end = std::min(order.size(), start + opts.batch);
            model.zero_grad();
            for (size_t k = start; k < end; ++k) {
                window(data, order[k], model.config().context, ctx.data());
                total += model.backward(ctx.data(), data[order[k]]);
            }
            model.step(opts.lr / float(end - start));
        }
        mean = float(total / double(data.size()));
        if (on_epoch) on_epoch(epoch, mean);
    }
    return mean;
}

}  // namespace child
