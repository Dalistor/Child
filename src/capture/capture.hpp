#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace child {

struct CaptureOptions {
    size_t max_file_bytes = 1 << 20;  // larger files are skipped
};

// Turns a source into the byte stream the model learns from. `path` may be a
// file, a directory (walked recursively in sorted order, skipping hidden
// folders, build output and binary files) or "-" for stdin. Each file is
// followed by a 0 byte, which is also the model's padding byte, so the context
// resets at file boundaries.
std::vector<uint8_t> capture(const std::string& path, const CaptureOptions& opts = {});

}  // namespace child
