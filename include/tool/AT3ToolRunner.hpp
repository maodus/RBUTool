#pragma once

#include "tool/ExeRunner.hpp"

class AT3ToolRunner : public ExeRunner {
 public:
  AT3ToolRunner(const std::string exe_path);
  ExeResult WavToAt3(const std::string& in_file, const std::string& out_file);
};