#pragma once
#include <core/Common.h>

#include <unordered_map>
#include <vector>

#include "core/chart/ChartDocument.hpp"

struct PostConfig {
  bool do_pruning{false};
  bool do_mgem_reduction{false};
  bool do_sparse_mgem{false};

  float prune_window{0.0f};
};

struct PostContext {
  const core::chart::ChartDocument& chart;
  TrackDifficulty diff;
  GameTrack track;
  std::unordered_map<int, const NoteIR*> note_map;
  const PostConfig& cfg;
};

class IPostPass {
 public:
  virtual ~IPostPass() = default;
  virtual std::vector<core::chart::ViewNote> Run(
      const PostContext& ctx,
      std::vector<core::chart::ViewNote>& notes) const = 0;

 private:
};