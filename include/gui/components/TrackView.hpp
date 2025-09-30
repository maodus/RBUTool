#pragma once
#include "gui/adapters/TrackViewContext.hpp"
namespace gui {
class TrackView {
 public:
  TrackView(float lane_height, const gui::TrackViewCtx& ctx);
  void RenderTrack();

 private:
  float lane_h_;
  const gui::TrackViewCtx& ctx_;

  void SelectRange(int selected_tick) const;
  const std::vector<core::chart::ViewNote>& GetOrBuildCache(int diff,
                                                            int track);
};
}  // namespace gui