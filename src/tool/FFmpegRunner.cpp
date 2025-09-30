#include "tool/FFmpegRunner.hpp"

#include <sstream>

FFmpegRunner::FFmpegRunner(const std::string exe_path) : ExeRunner(exe_path) {}

ExeResult FFmpegRunner::AudioToWav(const std::string& in_file,
                                   const std::string& out_file) {
  std::vector<std::string> args{"-y",
                                "-hide_banner",
                                "-i",
                                "\"" + in_file + "\"",
                                "-vn",
                                "-map",
                                "0:a:0?",
                                "-ac",
                                "1",
                                "-af",
                                "\"aresample=resampler=soxr\"",
                                "-ar",
                                "44100",
                                "-c:a",
                                "pcm_s16le",
                                "\"" + out_file + "\""};

  return this->Run(args);
}

inline std::string BuildMixFilter(size_t n) {
  std::ostringstream fc;
  for (size_t i = 0; i < n; ++i) {
    // Force mono + s16 and 44.1k per input, regardless of original layout
    fc << "[" << i << ":a]"
       << "aformat=sample_fmts=s16:channel_layouts=mono,"
       << "aresample=resampler=soxr:osr=44100"
       << "[a" << i << "];";
  }
  for (size_t i = 0; i < n; ++i) fc << "[a" << i << "]";
  fc << "amix=inputs=" << n
     << ":duration=longest:dropout_transition=0:normalize=0"
     << "[mix]";
  return fc.str();
}

ExeResult FFmpegRunner::MixToWav(std::vector<std::string> in_files,
                                 const std::string& out_file) {
  std::vector<std::string> args{"-y", "-hide_banner"};

  for (auto& i : in_files) {
    args.push_back("-i");
    args.push_back("\"" + i + "\"");
  }

  const std::string filter = BuildMixFilter(in_files.size());
  args.push_back("-filter_complex");
  args.push_back(filter);
  args.push_back("-map");
  args.push_back("[mix]");

  // Output format (belt & suspenders; should already be mono/44.1k)
  args.push_back("-c:a");
  args.push_back("pcm_s16le");
  args.push_back("-ar");
  args.push_back("44100");
  args.push_back("-ac");
  args.push_back("1");

  args.push_back(out_file);

  return this->Run(args);
}
