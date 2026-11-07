#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "model/model.hpp"

namespace child {

// Where a network came from. A fork keeps the parent's weights but gets its
// own id, so branches can diverge and still be traced back to the same root.
struct Lineage {
    uint64_t id = 0;
    uint64_t parent = 0;  // 0 for a root
    uint64_t steps = 0;   // bytes learned so far
    std::string name;
};

// What a base is made of: either the MLP (config + params), the oracle of
// the tests, or the brain, whose section is an opaque block that only
// src/brain/ reads. The engram stores sections; it never interprets the brain.
struct Snapshot {
    Config config;
    Lineage lineage;
    std::vector<float> params;
    std::vector<uint8_t> brain;  // non-empty for a brain base

    bool is_brain() const { return !brain.empty(); }
};

uint64_t new_id();

// Virgin snapshot: random weights, no history.
Snapshot create(const Config& cfg, uint64_t seed, std::string name);

// Same weights and history, new identity pointing back at `from`.
Snapshot fork(const Snapshot& from, std::string name);

// Engram format 2: lineage header, typed and versioned sections, checksum.
// Atomic (temp file + rename). Throws on any failure, naming unknown sections.
void save(const std::string& path, const Snapshot& snap);
Snapshot load(const std::string& path);

}  // namespace child
