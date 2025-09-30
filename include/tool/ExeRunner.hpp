#pragma once
#include <string>
#include <vector>

struct ExeResult {
  int exit_code;
  std::string log_out;
};

class ExeRunner {
 public:
  ExeRunner(const std::string exe_path);
  virtual ExeResult Run(const std::vector<std::string>& args);

 protected:
  const std::string path_;
};