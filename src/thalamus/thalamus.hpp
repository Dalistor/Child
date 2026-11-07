#pragma once

#include <cstdint>

#include "model/rng.hpp"

namespace child {

// A relay cell of the thalamus seen from the outside: it fires as a Poisson
// process at `rate_hz`. Intervals are drawn from the project's own Rng, so the
// same seed gives the same spikes on every machine, and the cost is one draw
// per spike instead of one per time step.
class SpikeSource {
public:
    SpikeSource(uint64_t seed, float rate_hz);

    // Takes effect from the next interval drawn.
    void set_rate(float rate_hz) { rate_ = rate_hz; }
    float rate() const { return rate_; }

    // Advances `dt` ms; returns how many spikes fell inside the step (a pooled
    // source standing for many cells can fire more than once per step).
    int step(float dt);

    // Draws the interval to a following spike, in ms (infinite at rate zero),
    // for callers that schedule spikes themselves instead of stepping.
    double interval();

private:
    Rng rng_;
    float rate_;
    double until_next_;  // ms
};

}  // namespace child
