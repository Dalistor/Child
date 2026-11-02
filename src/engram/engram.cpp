#include "store.hpp"

#include <bit>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <random>
#include <stdexcept>

static_assert(std::endian::native == std::endian::little, "checkpoint format is little-endian");

namespace child {

namespace {

constexpr char kMagic[4] = {'C', 'H', 'L', 'D'};
constexpr uint32_t kVersion = 1;

uint64_t fnv1a(const uint8_t* data, size_t n) {
    uint64_t h = 0xcbf29ce484222325ULL;
    for (size_t i = 0; i < n; ++i) h = (h ^ data[i]) * 0x100000001b3ULL;
    return h;
}

class Writer {
public:
    template <class T>
    void put(const T& v) {
        const auto* b = reinterpret_cast<const uint8_t*>(&v);
        buf.insert(buf.end(), b, b + sizeof(T));
    }
    void put_bytes(const void* p, size_t n) {
        const auto* b = static_cast<const uint8_t*>(p);
        buf.insert(buf.end(), b, b + n);
    }
    std::vector<uint8_t> buf;
};

class Reader {
public:
    Reader(const uint8_t* p, size_t n) : p_(p), end_(p + n) {}
    template <class T>
    T get() {
        T v;
        get_bytes(&v, sizeof(T));
        return v;
    }
    void get_bytes(void* out, size_t n) {
        if (size_t(end_ - p_) < n) throw std::runtime_error("checkpoint is truncated");
        std::memcpy(out, p_, n);
        p_ += n;
    }
    size_t remaining() const { return size_t(end_ - p_); }

private:
    const uint8_t* p_;
    const uint8_t* end_;
};

}  // namespace

uint64_t new_id() {
    std::random_device rd;
    uint64_t id = 0;
    while (id == 0) id = (uint64_t(rd()) << 32) | rd();
    return id;
}

Snapshot create(const Config& cfg, uint64_t seed, std::string name) {
    Model model(cfg, seed);
    return {cfg, {new_id(), 0, 0, std::move(name)}, model.params()};
}

Snapshot fork(const Snapshot& from, std::string name) {
    Snapshot out = from;
    out.lineage.parent = from.lineage.id;
    out.lineage.id = new_id();
    out.lineage.name = std::move(name);
    return out;
}

void save(const std::string& path, const Snapshot& snap) {
    if (snap.params.size() != snap.config.param_count())
        throw std::runtime_error("snapshot parameters do not match its config");

    Writer w;
    w.put_bytes(kMagic, sizeof kMagic);
    w.put(kVersion);
    w.put(snap.config.embed);
    w.put(snap.config.context);
    w.put(snap.config.hidden);
    w.put(snap.lineage.id);
    w.put(snap.lineage.parent);
    w.put(snap.lineage.steps);
    w.put(uint32_t(snap.lineage.name.size()));
    w.put_bytes(snap.lineage.name.data(), snap.lineage.name.size());
    w.put(uint64_t(snap.params.size()));
    w.put_bytes(snap.params.data(), snap.params.size() * sizeof(float));
    w.put(fnv1a(w.buf.data(), w.buf.size()));

    std::string tmp = path + ".tmp";
    {
        std::ofstream out(tmp, std::ios::binary | std::ios::trunc);
        out.write(reinterpret_cast<const char*>(w.buf.data()), std::streamsize(w.buf.size()));
        out.flush();
        if (!out) throw std::runtime_error("cannot write " + tmp);
    }
    std::filesystem::rename(tmp, path);
}

Snapshot load(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    if (!in) throw std::runtime_error("cannot open " + path);
    std::vector<uint8_t> buf{std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>()};

    if (buf.size() < sizeof kMagic + sizeof(uint64_t)) throw std::runtime_error("not a Child checkpoint: " + path);
    size_t body = buf.size() - sizeof(uint64_t);
    uint64_t stored;
    std::memcpy(&stored, buf.data() + body, sizeof stored);
    if (stored != fnv1a(buf.data(), body)) throw std::runtime_error("checkpoint is corrupted: " + path);

    Reader r(buf.data(), body);
    char magic[4];
    r.get_bytes(magic, sizeof magic);
    if (std::memcmp(magic, kMagic, sizeof magic) != 0) throw std::runtime_error("not a Child checkpoint: " + path);
    if (r.get<uint32_t>() != kVersion) throw std::runtime_error("unsupported checkpoint version");

    Snapshot s;
    s.config.embed = r.get<uint32_t>();
    s.config.context = r.get<uint32_t>();
    s.config.hidden = r.get<uint32_t>();
    s.lineage.id = r.get<uint64_t>();
    s.lineage.parent = r.get<uint64_t>();
    s.lineage.steps = r.get<uint64_t>();
    uint32_t name_len = r.get<uint32_t>();
    if (name_len > r.remaining()) throw std::runtime_error("checkpoint is truncated");
    s.lineage.name.resize(name_len);
    r.get_bytes(s.lineage.name.data(), name_len);

    uint64_t count = r.get<uint64_t>();
    if (count != s.config.param_count() || count * sizeof(float) != r.remaining())
        throw std::runtime_error("checkpoint parameters do not match its config");
    s.params.resize(count);
    r.get_bytes(s.params.data(), count * sizeof(float));
    return s;
}

}  // namespace child
