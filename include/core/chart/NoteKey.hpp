#pragma once
namespace core::chart {

struct NoteKey {
  int difficulty;
  int instrument;
  int id;

  bool operator==(const NoteKey& o) const {
    return difficulty == o.difficulty && instrument == o.instrument &&
           id == o.id;
  }
};

struct NoteKeyHash {
  size_t operator()(const NoteKey& k) const noexcept {
    // Simple 64-bit mix for 3 ints
    uint64_t x = (uint64_t)(uint32_t)k.difficulty;
    uint64_t y = (uint64_t)(uint32_t)k.instrument;
    uint64_t z = (uint64_t)(uint32_t)k.id;

    // Combine
    uint64_t h = x * 0x9E3779B185EBCA87ull;
    h ^= (y + 0x9E3779B185EBCA87ull + (h << 6) + (h >> 2));
    h ^= (z + 0x9E3779B185EBCA87ull + (h << 6) + (h >> 2));
    return (size_t)h;
  }
};

}  // namespace core::chart