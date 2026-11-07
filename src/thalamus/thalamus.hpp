#pragma once

#include <cstdint>
#include <vector>

#include "model/rng.hpp"

namespace child {

// A relay cell of the thalamus seen from the outside: it fires as a Poisson
// process at `rate_hz`. Intervals are drawn from the project's own Rng, so the
// same seed gives the same spikes on every machine, and the cost is one draw
// per spike instead of one per time step.
class SpikeSource {
public:
    SpikeSource(uint64_t seed, float rate_hz);

    // Takes effect at once: the pending interval is drawn again, which a
    // Poisson process allows because it has no memory.
    void set_rate(float rate_hz);
    float rate() const { return rate_; }

    // Advances `dt` ms; returns how many spikes fell inside the step (a pooled
    // source standing for many cells can fire more than once per step).
    int step(float dt);

    // Draws the interval to a following spike, in ms (infinite at rate zero),
    // for callers that schedule spikes themselves instead of stepping.
    double interval();

    template <class Archive>
    void persist(Archive& a) {
        rng_.persist(a);
        a(rate_);
        a(until_next_);
    }

private:
    Rng rng_;
    float rate_;
    double until_next_;  // ms
};

// The sensory relay: each byte value is a fixed sparse set of relay channels
// (a population code). While a byte is presented its channels fire at
// `active_rate`; the others keep a low spontaneous rate.
struct ThalamusParams {
    uint32_t channels = 256;         // relay cells
    uint32_t channels_per_byte = 12; // active for each byte value
    float active_rate = 300.f;       // Hz while its byte is presented (relay bursts reach a few hundred Hz)
    float spontaneous_rate = 2.f;    // Hz otherwise
};

class Thalamus {
public:
    Thalamus(const ThalamusParams& params, uint64_t seed);

    // Starts relaying `byte`: its channels switch to the active rate.
    void relay(uint8_t byte);
    // Advances `dt` ms; appends to `fired` one entry per spike (a channel can repeat).
    void step(float dt, std::vector<uint32_t>& fired);

    const std::vector<uint32_t>& code(uint8_t byte) const { return codes_[byte]; }
    uint32_t channels() const { return uint32_t(cells_.size()); }

    template <class Archive>
    void persist(Archive& a) {
        a(current_);
        a(cells_);
    }

private:
    ThalamusParams p_;
    std::vector<std::vector<uint32_t>> codes_;  // 256 sets of channel indices
    std::vector<SpikeSource> cells_;
    int32_t current_ = -1;  // byte being relayed, -1 before the first
};

}  // namespace child
