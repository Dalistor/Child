#pragma once

namespace child {

// Global dopamine signal, the third factor of plasticity. Times in ms.
// Releases are phasic pulses that decay with tau; there is no tonic baseline,
// so without a release the level is exactly zero.
class Dopamine {
public:
    explicit Dopamine(float tau = 200.f);  // tau_d, ms (Izhikevich 2007)

    // `amount` is the pulse's total over time: the level jumps by amount / tau,
    // so a unit release, fully decayed, integrates to one.
    void release(float amount);
    void step(float dt);

    float level() const { return level_; }  // per ms

private:
    float tau_;
    float level_ = 0.f;
};

}  // namespace child
