#pragma once

#include "tool/ExeRunner.hpp"

class FFmpegRunner : public ExeRunner {
 public:
  FFmpegRunner(const std::string exe_path);
  ExeResult AudioToWav(const std::string& in_file, const std::string& out_file);
  ExeResult MixToWav(std::vector<std::string> in_files,
                     const std::string& out_file);
};