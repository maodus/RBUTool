#pragma once
#include <cstdint>
#include <cstring>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>
#include <algorithm>

class BinaryWriter {
public:
    explicit BinaryWriter(const std::string& path)
        : _out(path, std::ios::binary) {
        if (!_out) throw std::runtime_error("open write: " + path);
    }

    void U32(uint32_t v) { Put32(v); }
    void U16(uint16_t v) { Put16(v); }
    void U8(uint8_t v) { _out.put(static_cast<char>(v)); }
    void F32(float v) { uint32_t fv; std::memcpy(&fv, &v, 4); Put32(fv); }
    void Bytes(const void* p, size_t n) { _out.write(reinterpret_cast<const char*>(p), std::streamsize(n)); }
    void Bytes(const std::vector<uint8_t>& v) { Bytes(v.data(), v.size()); }
    uint64_t Tell() { return static_cast<uint64_t>(_out.tellp()); }
    void Seek(uint64_t pos) { _out.seekp(std::streampos(pos)); }

private:
    std::ofstream _out;
    void Put32(uint32_t v) {
        _out.put(char(v & 0xFF));
        _out.put(char((v >> 8) & 0xFF));
        _out.put(char((v >> 16) & 0xFF));
        _out.put(char((v >> 24) & 0xFF));
    }

    void Put16(uint16_t v) {
        _out.put(char(v & 0xFF));
        _out.put(char((v >> 8) & 0xFF));
    }
};

struct IWritable {
    virtual ~IWritable() = default;
    virtual void write(BinaryWriter& w) const = 0;
};