#pragma once
#include <array>
#include <cstring>
#include <string>

namespace gui {

struct TextBuf {
  std::array<char, 1024> data{};

  bool Empty() const { return data[0] == '\0'; }
  void Set(const std::string& src) {
    std::strncpy(data.data(), src.c_str(), data.size() - 1);
    data.back() = '\0';
  }
  std::string ToString() const { return std::string(data.data()); }
};
}  // namespace gui