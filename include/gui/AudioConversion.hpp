#pragma once
#include <ImGuiFileDialog.h>
#include <core/Common.h>

#include <array>
#include <cstring>
#include <filesystem>
#include <string>
#include <vector>

#include "gui/common/FileInput.hpp"
namespace fs = std::filesystem;

struct UIPick {
  std::string label;
  gui::TextBuf input;

  UIPick() = default;
  explicit UIPick(std::string lbl) : label(std::move(lbl)) {}
};

struct AudioFilePick {
  UIPick main;
  std::vector<UIPick> parts;

  AudioFilePick() = default;
  explicit AudioFilePick(std::string mainLabel) : main(std::move(mainLabel)) {}

  void AddNumberedParts(std::string prefix, int count) {
    parts.reserve(parts.size() + count);
    for (int i = 1; i <= count; ++i)
      parts.emplace_back(prefix + "_" + std::to_string(i));
  }

  bool HasActiveParts() const {
    for (int i = 0; i < parts.size(); ++i) {
      if (!parts[i].input.Empty()) {
        return true;
      }
    }
    return false;
  }
};

struct FilePick {
  fs::path path;
  bool IsSet() const { return !path.empty(); }
};

struct GroupPick {
  FilePick main;
  std::vector<FilePick> parts;  // 0..N extra stems
  bool HasParts() const {
    for (auto& p : parts)
      if (p.IsSet()) return true;
    return false;
  }
  std::vector<fs::path> PartPaths() const {
    std::vector<fs::path> v;
    v.reserve(parts.size());
    for (auto& p : parts)
      if (p.IsSet()) v.push_back(p.path);
    return v;
  }
};

class AudioConversion {
 private:
  struct AudioEncryptionUI {
    AudioFilePick output_file{};
    std::vector<AudioFilePick> track_files;
    std::string last_dir = {"."};
  };

  AudioEncryptionUI enc_ui;

  void RenderAutoFill();
  void RenderMainFilePick(AudioFilePick& file_pick, ImGuiFileDialogFlags flags);
  void RenderPartFilePick(AudioFilePick& file_pick);
  void RenderOutput();

  void RenderEncryption();
  void RenderDecryption();

  void AutoFillFromDir(fs::path dir);

  std::vector<GroupPick> CollectInputs();

 public:
  AudioConversion();

  void Render();
};