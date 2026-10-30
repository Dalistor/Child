#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "model.hpp"

namespace child {

// Where a network came from. A fork keeps the parent's weights but gets its
// own id, so branches can diverge and still be traced back to the same root.
struct Lineage {
    uint64_t id = 0;
    uint64_t parent = 0;  // 0 for a root
    uint64_t steps = 0;   // bytes learned so far
    std::string name;
};

struct Snapshot {
    Config config;
    Lineage lineage;
    std::vector<float> params;
};

uint64_t new_id();

// Virgin snapshot: random weights, no history.
Snapshot create(const Config& cfg, uint64_t seed, std::string name);

// Same weights and history, new identity pointing back at `from`.
Snapshot fork(const Snapshot& from, std::string name);

// Atomic (temp file + rename) and checksummed. Throws on any failure.
void save(const std::string& path, const Snapshot& snap);
Snapshot load(const std::string& path);

}  // namespace child
