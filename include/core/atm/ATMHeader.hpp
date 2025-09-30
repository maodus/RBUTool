#pragma once
#include <stdint.h>

#include <array>

struct ATMHeader {
  uint32_t key4;            // 0x00
  uint32_t num_tracks;      // 0x04
  uint32_t key2;            // 0x08
  uint32_t chunk_size;      // 0x0C
  uint32_t key1;            // 0x10
  uint32_t track_bytes[5];  // 0x14 .. 0x27
  uint32_t word10;          // 0x28
  uint32_t word11;          // 0x2C
  uint32_t word12;          // 0x30
  uint32_t key3;            // 0x34

  uint32_t TotalTrackBytes() const;
  uint32_t GetLongestTrackBytes() const;
  std::array<uint32_t, 4> GetSecretKey() const;
  std::array<uint32_t, 4> GetIV(uint32_t track) const;

  static ATMHeader GetDefaultHeader();
};

static_assert(sizeof(ATMHeader) == 56);