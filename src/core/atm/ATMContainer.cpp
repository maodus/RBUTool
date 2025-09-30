#include "core/atm/ATMContainer.hpp"

#include <array>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <vector>

#include "core/atm/ATMHeader.hpp"
#include "crypto/HC128.hpp"

static void read_exact(std::ifstream& ifs, void* dst, std::size_t n) {
  if (!ifs.read(reinterpret_cast<char*>(dst), static_cast<std::streamsize>(n)))
    throw std::runtime_error("Unexpected EOF while reading");
}
static void write_exact(std::ofstream& ofs, const void* src, std::size_t n) {
  if (!ofs.write(reinterpret_cast<const char*>(src),
                 static_cast<std::streamsize>(n)))
    throw std::runtime_error("Write failed");
}

void ATMWriter::Write(std::array<std::string, 5> file_paths,
                      std::string out_file) {
  ATMHeader h = ATMHeader::GetDefaultHeader();

  // Load tracks
  struct Track {
    std::vector<std::uint8_t> data;
    std::uint32_t written = 0;
  };
  std::array<Track, 5> tracks;

  std::uint32_t totalBytes = 0;
  std::uint32_t maxLen = 0;

  for (std::size_t i = 0; i < 5; i++) {
    std::ifstream in(file_paths[i], std::ios::binary);
    if (!in) throw std::runtime_error("Missing input track: " + file_paths[i]);
    in.seekg(0, std::ios::end);
    const auto len = static_cast<std::uint32_t>(in.tellg());
    in.seekg(0, std::ios::beg);
    tracks[i].data.resize(len);
    if (len) read_exact(in, tracks[i].data.data(), len);
    h.track_bytes[i] = len;
    totalBytes += len;
    maxLen = std::max(maxLen, len);
  }

  // Prepare key/ivs
  const auto keyWords = h.GetSecretKey();
  std::array<std::uint8_t, 16> keyBytes{};
  std::memcpy(keyBytes.data(), keyWords.data(), 16);

  for (std::size_t i = 0; i < 5; i++) {
    auto& d = tracks[i].data;
    if (d.size() >= 8) {
      auto* w = reinterpret_cast<std::uint32_t*>(d.data());
      w[0] ^= totalBytes;
      w[1] ^= totalBytes;
    }
    const auto iv = h.GetIV(static_cast<std::uint32_t>(i));
    crypto::hc128(keyBytes.data(),
                  reinterpret_cast<const std::uint8_t*>(iv.data()), d.data(),
                  d.data(), d.size());
  }

  // Write ATM
  std::ofstream out(out_file, std::ios::binary | std::ios::trunc);
  if (!out) throw std::runtime_error("Cannot create " + out_file);
  write_exact(out, &h, sizeof(h));

  const uint32_t chunkSize = 400;
  // stripes: how many chunk rows we need for the longest track
  const std::uint32_t stripes = (maxLen + chunkSize - 1) / chunkSize;

  // Single fixed-size buffer for writing physical chunks
  std::vector<std::uint8_t> buf(chunkSize, 0);

  for (std::uint32_t s = 0; s < stripes; ++s) {
    for (std::size_t t = 0; t < 5; ++t) {
      auto& tr = tracks[t];

      // zero the whole physical block
      std::fill(buf.begin(), buf.end(), 0);

      if (tr.written < tr.data.size()) {
        const std::uint32_t remain =
            static_cast<std::uint32_t>(tr.data.size() - tr.written);
        const std::uint32_t take = std::min(remain, chunkSize);
        // copy only the logical bytes for this stripe
        std::memcpy(buf.data(), tr.data.data() + tr.written, take);
        tr.written += take;
      }

      // ALWAYS write chunkSize bytes physically
      write_exact(out, buf.data(), chunkSize);
    }
  }

  std::cout << "Encoded " << out_file << " (chunk=" << chunkSize
            << ", total=" << totalBytes << ")\n";
}