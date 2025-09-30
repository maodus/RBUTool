#include <algorithm>
#include <core/atm/ATMHeader.hpp>

uint32_t ATMHeader::TotalTrackBytes() const {
  uint32_t total_bytes = 0;
  for (int i = 0; i < 5; ++i) {
    total_bytes += this->track_bytes[i];
  }
  return total_bytes;
}

uint32_t ATMHeader::GetLongestTrackBytes() const {
  uint32_t max_bytes = 0;
  for (int i = 0; i < 5; ++i) {
    max_bytes = std::max(max_bytes, this->track_bytes[i]);
  }
  return max_bytes;
}

std::array<uint32_t, 4> ATMHeader::GetSecretKey() const {
  return {
      key1 ^ 0x5156B456u,
      key2 ^ 0x28CBD70Bu,
      key3 ^ 0x4B5AEFFCu,
      key4 ^ 0x6165E84Bu,
  };
}
std::array<uint32_t, 4> ATMHeader::GetIV(uint32_t track) const {
  // if (track >= h.num_tracks) throw std::out_of_range("track index");
  const uint32_t num_tracks = this->num_tracks;
  const uint32_t unk = ((8 - num_tracks) * 4) / num_tracks + 1;

  std::array<uint8_t, sizeof(ATMHeader)> header_bytes{};
  std::memcpy(header_bytes.data(), this, sizeof(ATMHeader));

  std::array<uint8_t, 16> iv{};
  for (int i = 0; i < 16; ++i) {
    const auto idx = (track * unk + (i % unk)) - track + num_tracks * 4 + 0x14;
    const std::uint8_t atm_byte = header_bytes.at(idx);
    const std::uint8_t last = (i == 0) ? 0 : iv[i - 1];
    iv[i] = static_cast<uint8_t>(atm_byte ^ last + ~atm_byte + uint8_t(i) +
                                                uint8_t(track));
  }
  std::array<uint32_t, 4> word_iv{};
  std::memcpy(word_iv.data(), iv.data(), iv.size());
  return word_iv;
}

ATMHeader ATMHeader::GetDefaultHeader() {
  ATMHeader header{};
  header.num_tracks = 5;
  header.chunk_size = 400;

  header.key1 = 0xDEADBEEFu;
  header.key2 = 0xDEADBEEFu;
  header.key3 = 0xDEADBEEFu;
  header.key4 = 0xDEADBEEFu;

  header.word10 = 0xDEADBEEF;
  header.word11 = 0xDEADBEEF;
  header.word12 = 0xDEADBEEF;

  for (int i = 0; i < 5; ++i) {
    header.track_bytes[i] = 0;
  }

  return header;
}
