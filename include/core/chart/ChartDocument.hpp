#pragma once
#include <vector>

#include "core/ChartIR.hpp"
#include "core/chart/ViewNote.hpp"
namespace core::chart {

struct TrackViewCache {
  bool dirty{true};  // set true on edits or toggle changes
  std::vector<core::chart::ViewNote> cached_view;  // last built view
};

struct ChartDocument {
  ChartIR working_chart;
  ChartIR base_chart;
  TrackViewCache track_cache[4][4];

  void DirtyInstrumentCache(uint32_t diff, uint32_t instrument) {
    track_cache[diff][instrument].dirty = true;
  };

  void DirtyTrackDiffCache(uint32_t diff) {
    for (uint32_t i = 0; i < 4; ++i) {
      DirtyInstrumentCache(diff, i);
    }
  };

  void DirtyAllTrackCache() {
    for (uint32_t i = 0; i < 4; ++i) {
      DirtyTrackDiffCache(i);
    }
  };
};

}  // namespace core::chart