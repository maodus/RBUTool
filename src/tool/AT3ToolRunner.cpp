#include "tool/AT3ToolRunner.hpp"

AT3ToolRunner::AT3ToolRunner(const std::string exe_path)
    : ExeRunner(exe_path) {}

ExeResult AT3ToolRunner::WavToAt3(const std::string& in_file,
                                  const std::string& out_file) {
  std::vector<std::string> args{"-e", "-br", "66", "\"" + in_file + "\"",
                                "\"" + out_file + "\""};
  return this->Run(args);
}
