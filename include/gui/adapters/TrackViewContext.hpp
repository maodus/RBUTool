#pragma once

#include <core/post/PostPass.hpp>

#include "core/chart/ChartDocument.hpp"
#include "core/chart/Timeline.hpp"
#include "core/selection/SelectionManager.hpp"

namespace gui {

struct TrackViewCtx {
  int difficulty;
  int instrument;
  core::chart::ChartDocument& chart_doc;
  BeatTimeline& timing;
  core::selection::SelectionManager& sel_man;
  PostConfig& post_conf;
};

}  // namespace gui