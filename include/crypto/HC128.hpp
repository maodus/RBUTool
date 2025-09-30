#pragma once
#include <cstddef>
#include <cstdint>

namespace crypto {
struct HC128State;  // forward decl

void initialize(HC128State& st, const std::uint8_t key[16],
                const std::uint8_t iv[16]);
void crypt(HC128State& st, const std::uint8_t* in, std::uint8_t* out,
           std::size_t len);
void hc128(const std::uint8_t key[16], const std::uint8_t iv[16],
           const std::uint8_t* in, std::uint8_t* out, std::size_t len);
}  // namespace crypto