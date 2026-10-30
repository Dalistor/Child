#include "capture.hpp"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>

namespace fs = std::filesystem;

namespace child {

namespace {

bool skipped_dir(const std::string& name) {
    return name.starts_with('.') || name == "build" || name == "node_modules";
}

bool looks_binary(const std::vector<uint8_t>& data) {
    size_t n = std::min<size_t>(data.size(), 4096);
    return std::find(data.begin(), data.begin() + n, 0) != data.begin() + n;
}

std::vector<uint8_t> read_file(const fs::path& p) {
    std::ifstream in(p, std::ios::binary);
    if (!in) throw std::runtime_error("cannot open " + p.string());
    return {std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>()};
}

void append_file(std::vector<uint8_t>& out, const fs::path& p, const CaptureOptions& opts) {
    std::error_code ec;
    if (fs::file_size(p, ec) > opts.max_file_bytes || ec) return;
    std::vector<uint8_t> data = read_file(p);
    if (looks_binary(data)) return;
    out.insert(out.end(), data.begin(), data.end());
    out.push_back(0);
}

}  // namespace

std::vector<uint8_t> capture(const std::string& path, const CaptureOptions& opts) {
    std::vector<uint8_t> out;

    if (path == "-") {
        out.assign(std::istreambuf_iterator<char>(std::cin), std::istreambuf_iterator<char>());
        out.push_back(0);
        return out;
    }

    if (!fs::exists(path)) throw std::runtime_error("no such path: " + path);

    if (fs::is_regular_file(path)) {
        append_file(out, path, {SIZE_MAX});
        return out;
    }

    std::vector<fs::path> files;
    for (fs::recursive_directory_iterator it(path), end; it != end; ++it) {
        if (it->is_directory() && skipped_dir(it->path().filename().string())) {
            it.disable_recursion_pending();
        } else if (it->is_regular_file()) {
            files.push_back(it->path());
        }
    }
    std::sort(files.begin(), files.end());
    for (const fs::path& f : files) append_file(out, f, opts);
    return out;
}

}  // namespace child
