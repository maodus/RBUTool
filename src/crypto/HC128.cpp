#include "crypto/HC128.hpp"

#include <array>
#include <cstdint>
#include <cstring>

namespace crypto {

// Direct C++ port of the original reference. Kept verbatim in spirit.
struct alignas(16) HC128State {
  std::uint32_t P[512]{};
  std::uint32_t Q[512]{};
  std::uint32_t keystream[16]{};
  std::uint32_t counter1024{};
};

static inline std::uint32_t rotr(std::uint32_t x, unsigned n) {
  return (x >> n) | (x << (32 - n));
}
static inline std::uint32_t rotl(std::uint32_t x, unsigned n) {
  return (x << n) | (x >> (32 - n));
}

static inline void h1(const HC128State& s, std::uint32_t x, std::uint32_t& y) {
  std::uint8_t a = static_cast<std::uint8_t>(x);
  std::uint8_t c = static_cast<std::uint8_t>(x >> 16);
  y = s.Q[a] + s.Q[256 + c];
}
static inline void h2(const HC128State& s, std::uint32_t x, std::uint32_t& y) {
  std::uint8_t a = static_cast<std::uint8_t>(x);
  std::uint8_t c = static_cast<std::uint8_t>(x >> 16);
  y = s.P[a] + s.P[256 + c];
}

static void sixteenSteps(HC128State& s) {
  std::uint32_t t0, t1, t2, t3;
  std::uint32_t cc = s.counter1024 & 0x1ff;
  std::uint32_t dd = (cc + 16) & 0x1ff;
  std::uint32_t ee = (cc - 16) & 0x1ff;

  auto step_P = [&](std::uint32_t& m0, std::uint32_t m511, std::uint32_t m3,
                    std::uint32_t m10, std::uint32_t m12, std::uint32_t& out) {
    t0 = rotr(m511, 23);
    t1 = rotr(m3, 10);
    t2 = rotr(m10, 8);
    m0 += t2 + (t0 ^ t1);
    h1(s, m12, t3);
    out = t3 ^ m0;
  };
  auto step_Q = [&](std::uint32_t& m0, std::uint32_t m511, std::uint32_t m3,
                    std::uint32_t m10, std::uint32_t m12, std::uint32_t& out) {
    t0 = rotl(m511, 23);
    t1 = rotl(m3, 10);
    t2 = rotl(m10, 8);
    m0 += t2 + (t0 ^ t1);
    h2(s, m12, t3);
    out = t3 ^ m0;
  };

  if (s.counter1024 < 512) {
    step_P(s.P[cc + 0], s.P[cc + 1], s.P[ee + 13], s.P[ee + 6], s.P[ee + 4],
           s.keystream[0]);
    step_P(s.P[cc + 1], s.P[cc + 2], s.P[ee + 14], s.P[ee + 7], s.P[ee + 5],
           s.keystream[1]);
    step_P(s.P[cc + 2], s.P[cc + 3], s.P[ee + 15], s.P[ee + 8], s.P[ee + 6],
           s.keystream[2]);
    step_P(s.P[cc + 3], s.P[cc + 4], s.P[cc + 0], s.P[ee + 9], s.P[ee + 7],
           s.keystream[3]);
    step_P(s.P[cc + 4], s.P[cc + 5], s.P[cc + 1], s.P[ee + 10], s.P[ee + 8],
           s.keystream[4]);
    step_P(s.P[cc + 5], s.P[cc + 6], s.P[cc + 2], s.P[ee + 11], s.P[ee + 9],
           s.keystream[5]);
    step_P(s.P[cc + 6], s.P[cc + 7], s.P[cc + 3], s.P[ee + 12], s.P[ee + 10],
           s.keystream[6]);
    step_P(s.P[cc + 7], s.P[cc + 8], s.P[cc + 4], s.P[ee + 13], s.P[ee + 11],
           s.keystream[7]);
    step_P(s.P[cc + 8], s.P[cc + 9], s.P[cc + 5], s.P[ee + 14], s.P[ee + 12],
           s.keystream[8]);
    step_P(s.P[cc + 9], s.P[cc + 10], s.P[cc + 6], s.P[ee + 15], s.P[ee + 13],
           s.keystream[9]);
    step_P(s.P[cc + 10], s.P[cc + 11], s.P[cc + 7], s.P[cc + 0], s.P[ee + 14],
           s.keystream[10]);
    step_P(s.P[cc + 11], s.P[cc + 12], s.P[cc + 8], s.P[cc + 1], s.P[ee + 15],
           s.keystream[11]);
    step_P(s.P[cc + 12], s.P[cc + 13], s.P[cc + 9], s.P[cc + 2], s.P[cc + 0],
           s.keystream[12]);
    step_P(s.P[cc + 13], s.P[cc + 14], s.P[cc + 10], s.P[cc + 3], s.P[cc + 1],
           s.keystream[13]);
    step_P(s.P[cc + 14], s.P[cc + 15], s.P[cc + 11], s.P[cc + 4], s.P[cc + 2],
           s.keystream[14]);
    step_P(s.P[cc + 15], s.P[dd + 0], s.P[cc + 12], s.P[cc + 5], s.P[cc + 3],
           s.keystream[15]);
  } else {
    step_Q(s.Q[cc + 0], s.Q[cc + 1], s.Q[ee + 13], s.Q[ee + 6], s.Q[ee + 4],
           s.keystream[0]);
    step_Q(s.Q[cc + 1], s.Q[cc + 2], s.Q[ee + 14], s.Q[ee + 7], s.Q[ee + 5],
           s.keystream[1]);
    step_Q(s.Q[cc + 2], s.Q[cc + 3], s.Q[ee + 15], s.Q[ee + 8], s.Q[ee + 6],
           s.keystream[2]);
    step_Q(s.Q[cc + 3], s.Q[cc + 4], s.Q[cc + 0], s.Q[ee + 9], s.Q[ee + 7],
           s.keystream[3]);
    step_Q(s.Q[cc + 4], s.Q[cc + 5], s.Q[cc + 1], s.Q[ee + 10], s.Q[ee + 8],
           s.keystream[4]);
    step_Q(s.Q[cc + 5], s.Q[cc + 6], s.Q[cc + 2], s.Q[ee + 11], s.Q[ee + 9],
           s.keystream[5]);
    step_Q(s.Q[cc + 6], s.Q[cc + 7], s.Q[cc + 3], s.Q[ee + 12], s.Q[ee + 10],
           s.keystream[6]);
    step_Q(s.Q[cc + 7], s.Q[cc + 8], s.Q[cc + 4], s.Q[ee + 13], s.Q[ee + 11],
           s.keystream[7]);
    step_Q(s.Q[cc + 8], s.Q[cc + 9], s.Q[cc + 5], s.Q[ee + 14], s.Q[ee + 12],
           s.keystream[8]);
    step_Q(s.Q[cc + 9], s.Q[cc + 10], s.Q[cc + 6], s.Q[ee + 15], s.Q[ee + 13],
           s.keystream[9]);
    step_Q(s.Q[cc + 10], s.Q[cc + 11], s.Q[cc + 7], s.Q[cc + 0], s.Q[ee + 14],
           s.keystream[10]);
    step_Q(s.Q[cc + 11], s.Q[cc + 12], s.Q[cc + 8], s.Q[cc + 1], s.Q[ee + 15],
           s.keystream[11]);
    step_Q(s.Q[cc + 12], s.Q[cc + 13], s.Q[cc + 9], s.Q[cc + 2], s.Q[cc + 0],
           s.keystream[12]);
    step_Q(s.Q[cc + 13], s.Q[cc + 14], s.Q[cc + 10], s.Q[cc + 3], s.Q[cc + 1],
           s.keystream[13]);
    step_Q(s.Q[cc + 14], s.Q[cc + 15], s.Q[cc + 11], s.Q[cc + 4], s.Q[cc + 2],
           s.keystream[14]);
    step_Q(s.Q[cc + 15], s.Q[dd + 0], s.Q[cc + 12], s.Q[cc + 5], s.Q[cc + 3],
           s.keystream[15]);
  }
  s.counter1024 = (s.counter1024 + 16) & 0x3ff;
}

static inline std::uint32_t f1(std::uint32_t x) {
  return rotr(x, 7) ^ rotr(x, 18) ^ (x >> 3);
}
static inline std::uint32_t f2(std::uint32_t x) {
  return rotr(x, 17) ^ rotr(x, 19) ^ (x >> 10);
}
static inline std::uint32_t ff(std::uint32_t a, std::uint32_t b,
                               std::uint32_t c, std::uint32_t d) {
  return f2(a) + b + f1(c) + d;
}

static void updateSixteen(HC128State& s) {
  std::uint32_t t0, t1, t2, t3;
  std::uint32_t cc = s.counter1024 & 0x1ff;
  std::uint32_t dd = (cc + 16) & 0x1ff;
  std::uint32_t ee = (cc - 16) & 0x1ff;
  auto upd_P = [&](std::uint32_t& m0, std::uint32_t m511, std::uint32_t m3,
                   std::uint32_t m10, std::uint32_t m12) {
    t0 = rotr(m511, 23);
    t1 = rotr(m3, 10);
    t2 = rotr(m10, 8);
    m0 += t2 + (t0 ^ t1);
    h1(s, m12, t3);
    m0 = t3 ^ m0;
  };
  auto upd_Q = [&](std::uint32_t& m0, std::uint32_t m511, std::uint32_t m3,
                   std::uint32_t m10, std::uint32_t m12) {
    t0 = rotl(m511, 23);
    t1 = rotl(m3, 10);
    t2 = rotl(m10, 8);
    m0 += t2 + (t0 ^ t1);
    h2(s, m12, t3);
    m0 = t3 ^ m0;
  };

  if (s.counter1024 < 512) {
    upd_P(s.P[cc + 0], s.P[cc + 1], s.P[ee + 13], s.P[ee + 6], s.P[ee + 4]);
    upd_P(s.P[cc + 1], s.P[cc + 2], s.P[ee + 14], s.P[ee + 7], s.P[ee + 5]);
    upd_P(s.P[cc + 2], s.P[cc + 3], s.P[ee + 15], s.P[ee + 8], s.P[ee + 6]);
    upd_P(s.P[cc + 3], s.P[cc + 4], s.P[cc + 0], s.P[ee + 9], s.P[ee + 7]);
    upd_P(s.P[cc + 4], s.P[cc + 5], s.P[cc + 1], s.P[ee + 10], s.P[ee + 8]);
    upd_P(s.P[cc + 5], s.P[cc + 6], s.P[cc + 2], s.P[ee + 11], s.P[ee + 9]);
    upd_P(s.P[cc + 6], s.P[cc + 7], s.P[cc + 3], s.P[ee + 12], s.P[ee + 10]);
    upd_P(s.P[cc + 7], s.P[cc + 8], s.P[cc + 4], s.P[ee + 13], s.P[ee + 11]);
    upd_P(s.P[cc + 8], s.P[cc + 9], s.P[cc + 5], s.P[ee + 14], s.P[ee + 12]);
    upd_P(s.P[cc + 9], s.P[cc + 10], s.P[cc + 6], s.P[ee + 15], s.P[ee + 13]);
    upd_P(s.P[cc + 10], s.P[cc + 11], s.P[cc + 7], s.P[cc + 0], s.P[ee + 14]);
    upd_P(s.P[cc + 11], s.P[cc + 12], s.P[cc + 8], s.P[cc + 1], s.P[ee + 15]);
    upd_P(s.P[cc + 12], s.P[cc + 13], s.P[cc + 9], s.P[cc + 2], s.P[cc + 0]);
    upd_P(s.P[cc + 13], s.P[cc + 14], s.P[cc + 10], s.P[cc + 3], s.P[cc + 1]);
    upd_P(s.P[cc + 14], s.P[cc + 15], s.P[cc + 11], s.P[cc + 4], s.P[cc + 2]);
    upd_P(s.P[cc + 15], s.P[dd + 0], s.P[cc + 12], s.P[cc + 5], s.P[cc + 3]);
  } else {
    upd_Q(s.Q[cc + 0], s.Q[cc + 1], s.Q[ee + 13], s.Q[ee + 6], s.Q[ee + 4]);
    upd_Q(s.Q[cc + 1], s.Q[cc + 2], s.Q[ee + 14], s.Q[ee + 7], s.Q[ee + 5]);
    upd_Q(s.Q[cc + 2], s.Q[cc + 3], s.Q[ee + 15], s.Q[ee + 8], s.Q[ee + 6]);
    upd_Q(s.Q[cc + 3], s.Q[cc + 4], s.Q[cc + 0], s.Q[ee + 9], s.Q[ee + 7]);
    upd_Q(s.Q[cc + 4], s.Q[cc + 5], s.Q[cc + 1], s.Q[ee + 10], s.Q[ee + 8]);
    upd_Q(s.Q[cc + 5], s.Q[cc + 6], s.Q[cc + 2], s.Q[ee + 11], s.Q[ee + 9]);
    upd_Q(s.Q[cc + 6], s.Q[cc + 7], s.Q[cc + 3], s.Q[ee + 12], s.Q[ee + 10]);
    upd_Q(s.Q[cc + 7], s.Q[cc + 8], s.Q[cc + 4], s.Q[ee + 13], s.Q[ee + 11]);
    upd_Q(s.Q[cc + 8], s.Q[cc + 9], s.Q[cc + 5], s.Q[ee + 14], s.Q[ee + 12]);
    upd_Q(s.Q[cc + 9], s.Q[cc + 10], s.Q[cc + 6], s.Q[ee + 15], s.Q[ee + 13]);
    upd_Q(s.Q[cc + 10], s.Q[cc + 11], s.Q[cc + 7], s.Q[cc + 0], s.Q[ee + 14]);
    upd_Q(s.Q[cc + 11], s.Q[cc + 12], s.Q[cc + 8], s.Q[cc + 1], s.Q[ee + 15]);
    upd_Q(s.Q[cc + 12], s.Q[cc + 13], s.Q[cc + 9], s.Q[cc + 2], s.Q[cc + 0]);
    upd_Q(s.Q[cc + 13], s.Q[cc + 14], s.Q[cc + 10], s.Q[cc + 3], s.Q[cc + 1]);
    upd_Q(s.Q[cc + 14], s.Q[cc + 15], s.Q[cc + 11], s.Q[cc + 4], s.Q[cc + 2]);
    upd_Q(s.Q[cc + 15], s.Q[dd + 0], s.Q[cc + 12], s.Q[cc + 5], s.Q[cc + 3]);
  }
  s.counter1024 = (s.counter1024 + 16) & 0x3ff;
}

static void initialize_internal(HC128State& s, const std::uint8_t key[16],
                                const std::uint8_t iv[16]) {
  // Expand key/iv
  for (int i = 0; i < 4; i++) {
    s.P[i] = reinterpret_cast<const std::uint32_t*>(key)[i];
    s.P[i + 4] = reinterpret_cast<const std::uint32_t*>(key)[i];
  }
  for (int i = 0; i < 4; i++) {
    s.P[i + 8] = reinterpret_cast<const std::uint32_t*>(iv)[i];
    s.P[i + 12] = reinterpret_cast<const std::uint32_t*>(iv)[i];
  }

  for (int i = 16; i < 256 + 16; i++)
    s.P[i] = ff(s.P[i - 2], s.P[i - 7], s.P[i - 15], s.P[i - 16]) +
             static_cast<std::uint32_t>(i);
  for (int i = 0; i < 16; i++) s.P[i] = s.P[i + 256];
  for (int i = 16; i < 512; i++)
    s.P[i] = ff(s.P[i - 2], s.P[i - 7], s.P[i - 15], s.P[i - 16]) +
             static_cast<std::uint32_t>(256 + i);

  for (int i = 0; i < 16; i++) s.Q[i] = s.P[512 - 16 + i];
  for (int i = 16; i < 32; i++)
    s.Q[i] = ff(s.Q[i - 2], s.Q[i - 7], s.Q[i - 15], s.Q[i - 16]) +
             static_cast<std::uint32_t>(256 + 512 + (i - 16));
  for (int i = 0; i < 16; i++) s.Q[i] = s.Q[i + 16];
  for (int i = 16; i < 512; i++)
    s.Q[i] = ff(s.Q[i - 2], s.Q[i - 7], s.Q[i - 15], s.Q[i - 16]) +
             static_cast<std::uint32_t>(768 + i);

  s.counter1024 = 0;
  for (int i = 0; i < 64; i++) updateSixteen(s);  // warmup
}

void initialize(HC128State& st, const std::uint8_t key[16],
                const std::uint8_t iv[16]) {
  initialize_internal(st, key, iv);
}

void crypt(HC128State& st, const std::uint8_t* in, std::uint8_t* out,
           std::size_t len) {
  std::size_t i = 0;
  while (i + 64 <= len) {
    sixteenSteps(st);
    auto* ks = reinterpret_cast<const std::uint8_t*>(st.keystream);
    for (std::size_t j = 0; j < 64; j++)
      out[i + j] = static_cast<std::uint8_t>(in[i + j] ^ ks[j]);
    i += 64;
  }
  if (i < len) {
    sixteenSteps(st);
    auto* ks = reinterpret_cast<const std::uint8_t*>(st.keystream);
    for (std::size_t j = 0; j < (len - i); j++)
      out[i + j] = static_cast<std::uint8_t>(in[i + j] ^ ks[j]);
  }
}

void hc128(const std::uint8_t key[16], const std::uint8_t iv[16],
           const std::uint8_t* in, std::uint8_t* out, std::size_t len) {
  HC128State st;
  initialize(st, key, iv);
  crypt(st, in, out, len);
}

}  // namespace crypto