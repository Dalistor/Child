#pragma once

#include <cstdint>

namespace child {

// SplitMix64: small, fast and identical on every platform, so a seed always
// produces the same virgin network.
class Rng {
public:
    explicit Rng(uint64_t seed) : s_(seed) {}

    uint64_t next() {
        uint64_t z = (s_ += 0x9e3779b97f4a7c15ULL);
        z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
        z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
        return z ^ (z >> 31);
    }

    // Uniform in [0, 1).
    float uniform() { return float(next() >> 40) / float(1ULL << 24); }

private:
    uint64_t s_;
};

}  // namespace child
