#pragma once

#include <ImGuiFileDialog.h>

#include <algorithm>
#include <cmath>
#include <core/ChartIR.hpp>
#include <core/RbuProcessor.hpp>
#include <core/post/PostPass.hpp>
#include <unordered_set>
#include <vector>

#include "core/chart/ChartDocument.hpp"
#include "core/chart/ChartImporter.hpp"
#include "core/chart/Timeline.hpp"
#include "core/selection/SelectionManager.hpp"
#include "gui/components/TrackView.hpp"
#include "imgui.h"

class ChartWindow {
 private:
  std::unique_ptr<core::chart::IChartImporter> chart_importer;
  struct FileDialogContext {
    ImGuiFileDialog dialog;
    std::string last_path;
    std::array<char, 1024> path_buf{};
  };
  FileDialogContext fd_ctx;
  BeatTimeline bt;
  core::selection::SelectionManager sm;
  int difficulty = 0;
  float lane_h = 22.0f;  // lane row height (px)

  void RenderExport();
  void RenderImport();

  void RenderNoneSelected();
  void RenderChartSelected();

  void RenderSelectedInfo();
  void CollapseNotes();

 public:
  core::chart::ChartDocument cd;
  PostConfig pc;
  std::shared_ptr<RBUHeader> rbu_header;
  ChartWindow();

  void render();
  void RenderTrack(int track_id);
  void UpdateBaseLine();
  void FinalizeTrackCaches();
};