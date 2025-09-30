#pragma once

#include "ImGuiFileDialog.h"
#include "gui/common/TextBuf.hpp"

namespace gui {
struct FileInput {
  TextBuf text_buf;
  std::unique_ptr<ImGuiFileDialog> dialog;

  FileInput::FileInput() : dialog(std::make_unique<ImGuiFileDialog>()) {}
  FileInput::~FileInput() = default;
};
}  // namespace gui