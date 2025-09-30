#pragma once
#include <cstdint>

namespace core ::chart {

struct ViewNote {
  int id;
  char lane;
  int on, off;
  int modifiers;
  uint8_t velocity;
};

}  // namespace core::chart