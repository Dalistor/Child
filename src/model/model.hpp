#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <vector>

namespace child {

// Every byte is a token, so any text or code is representable.
constexpr uint32_t kVocab = 256;

struct Config {
    uint32_t embed = 32;    // size of each byte's embedding
    uint32_t context = 16;  // how many previous bytes the model sees
    uint32_t hidden = 128;  // width of the hidden layer

    size_t param_count() const;
    bool operator==(const Config&) const = default;
};

// Reads a window of bytes and predicts the next one:
//   embeddings(window) -> tanh(W1 x + b1) -> softmax(W2 h + b2)
// All parameters live in one flat vector so they can be stored, copied and
// forked as a single block.
class Model {
public:
    // Virgin network: random weights, nothing learned.
    Model(const Config& cfg, uint64_t seed);
    // Restore a network from stored parameters.
    Model(const Config& cfg, std::vector<float> params);

    const Config& config() const { return cfg_; }
    const std::vector<float>& params() const { return p_; }
    std::vector<float>& params() { return p_; }
    const std::vector<float>& grads() const { return g_; }

    // `ctx` points at config().context bytes. Fills `probs` (kVocab floats).
    void predict(const uint8_t* ctx, float* probs);

    // Cross-entropy loss for `target` following `ctx`; adds its gradient to
    // grads(). Call zero_grad() before a batch and step() after it.
    float backward(const uint8_t* ctx, uint8_t target);

    void zero_grad();
    // p -= lr * g
    void step(float lr);

private:
    void layout();
    void forward(const uint8_t* ctx);

    Config cfg_;
    std::vector<float> p_, g_;
    size_t o_emb_ = 0, o_w1_ = 0, o_b1_ = 0, o_w2_ = 0, o_b2_ = 0;

    // scratch buffers reused across calls
    std::vector<float> x_, dx_, h_, dh_, probs_;
};

struct LearnOptions {
    int epochs = 1;
    float lr = 0.05f;
    size_t batch = 32;
    uint64_t seed = 1;  // shuffling only; the network itself is untouched by it
};

// Fills `ctx` (config().context bytes) with what precedes position `pos` in
// `data`, padding with 0 before the start.
void window(const std::vector<uint8_t>& data, size_t pos, uint32_t context, uint8_t* ctx);

// Learns to predict every byte of `data` from the bytes before it. Returns the
// mean loss of the last epoch; `on_epoch(epoch, mean_loss)` is called after each.
float learn(Model& model, const std::vector<uint8_t>& data, const LearnOptions& opts,
            const std::function<void(int, float)>& on_epoch = {});

}  // namespace child
