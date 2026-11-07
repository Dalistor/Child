#pragma once

#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <type_traits>
#include <vector>

namespace child {

// Every brain piece exposes `template <class Archive> void persist(Archive&)`
// that names its fields once; the same function then saves and restores them.
// Values are copied as little-endian bytes (checked in engram.cpp), vectors
// with their length first.

class ByteWriter {
public:
    static constexpr bool loading = false;

    template <class T>
    void operator()(const T& v) {
        static_assert(std::is_arithmetic_v<T> || std::is_enum_v<T>);
        const auto* b = reinterpret_cast<const uint8_t*>(&v);
        bytes.insert(bytes.end(), b, b + sizeof(T));
    }
    template <class T>
    void operator()(const std::vector<T>& v) {
        (*this)(uint64_t(v.size()));
        if constexpr (std::is_arithmetic_v<T>) {
            const auto* b = reinterpret_cast<const uint8_t*>(v.data());
            bytes.insert(bytes.end(), b, b + v.size() * sizeof(T));
        } else {
            for (const T& x : v) const_cast<T&>(x).persist(*this);
        }
    }

    std::vector<uint8_t> bytes;
};

class ByteReader {
public:
    static constexpr bool loading = true;

    ByteReader(const uint8_t* data, size_t size) : p_(data), end_(data + size) {}

    template <class T>
    void operator()(T& v) {
        static_assert(std::is_arithmetic_v<T> || std::is_enum_v<T>);
        take(&v, sizeof(T));
    }
    template <class T>
    void operator()(std::vector<T>& v) {
        uint64_t n = 0;
        (*this)(n);
        if (n > remaining()) throw std::runtime_error("brain section is truncated");
        if constexpr (std::is_arithmetic_v<T>) {
            v.resize(size_t(n));
            take(v.data(), size_t(n) * sizeof(T));
        } else {
            // Elements already exist (built from the parameters); only their state is read.
            if (n != v.size()) throw std::runtime_error("brain section does not match its parameters");
            for (T& x : v) x.persist(*this);
        }
    }

    size_t remaining() const { return size_t(end_ - p_); }

private:
    void take(void* out, size_t n) {
        if (remaining() < n) throw std::runtime_error("brain section is truncated");
        std::memcpy(out, p_, n);
        p_ += n;
    }

    const uint8_t* p_;
    const uint8_t* end_;
};

}  // namespace child
