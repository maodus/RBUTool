#pragma once
#include <array>
#include <string>

class ATMWriter {
 public:
  void Write(std::array<std::string, 5> file_paths, std::string out_file);
};

class ATMReader {};