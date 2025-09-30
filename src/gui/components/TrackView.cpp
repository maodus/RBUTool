#include "gui/components/TrackView.hpp"

#include <imgui.h>

#include <core\post\PostPipeline.hpp>

#include "core/chart/NoteKey.hpp"
#include "core/selection/SelectionManager.hpp"
#include "util/BitUtils.h"

namespace gui {

static int next_id = 999991;

static inline bool RectContains(const ImVec2& p, const ImVec2& a,
                                const ImVec2& b) {
  return p.x >= a.x && p.y >= a.y && p.x <= b.x && p.y <= b.y;
}

static ImU32 lane_cols[4] = {
    IM_COL32(230, 90, 80, 255), IM_COL32(240, 190, 60, 255),
    IM_COL32(80, 200, 120, 255), IM_COL32(70, 140, 240, 255)};

TrackView::TrackView(float lane_height, const gui::TrackViewCtx& ctx)
    : lane_h_(lane_height), ctx_(ctx) {}

void TrackView::RenderTrack() {
  const ImU32 col_lane = ImGui::GetColorU32(ImVec4(1, 1, 1, 0.05f));
  const float view_height = lane_h_ * 4;

  ImGui::PushID(ctx_.instrument);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
  ImGui::BeginChild(
      "trackview", ImVec2(0, view_height), true,
      ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

  // Canvas and clip info
  ImDrawList* dl = ImGui::GetWindowDrawList();
  const ImVec2 canvas_min = ImGui::GetCursorScreenPos();
  const ImVec2 canvas_size = ImGui::GetContentRegionAvail();
  const ImVec2 canvas_max =
      ImVec2(canvas_min.x + canvas_size.x, canvas_min.y + canvas_size.y);
  const ImVec2 clip_min = ImGui::GetWindowDrawList()->GetClipRectMin();
  const ImVec2 clip_max = ImGui::GetWindowDrawList()->GetClipRectMax();

  // Draw horizontal lane separators/lines
  for (int i = 1; i < 4; ++i) {
    float y = canvas_min.y + i * lane_h_;
    dl->AddLine(ImVec2(clip_min.x, y), ImVec2(clip_max.x, y), col_lane, 1.0f);
  }

  int hovered = -1;
  const ImVec2 mouse = ImGui::GetIO().MousePos;

  auto& notes = GetOrBuildCache(ctx_.difficulty, ctx_.instrument);
  // auto& notes = ctx.song.tl.tracks.at(ctx.instrument).notes;

  for (int i = 0; i < (int)notes.size(); ++i) {
    const auto& note = notes[i];
    float x_min = ctx_.timing.TickToX(note.on, canvas_min.x);
    float x_max = ctx_.timing.TickToX(note.off, canvas_min.x);

    // Simple O(n) cull, improve in future.
    const bool cull_left = (x_min < clip_min.x) && (x_max < clip_min.x);
    const bool cull_right = (x_min > clip_max.x) && (x_max > clip_max.x);
    if (cull_left) continue;
    if (cull_right) break;

    core::chart::NoteKey nk = {ctx_.difficulty, ctx_.instrument, note.id};
    bool is_selected = ctx_.sel_man.IsSelected(nk);
    ImU32 borderCol =
        is_selected ? IM_COL32(255, 160, 60, 255) : IM_COL32(0, 0, 0, 255);
    float thick = is_selected ? 2.0f : 1.5f;

    for (int j = 0; j < 4; ++j) {
      int lane = ((note.lane >> j) & 1);
      if (!lane) {
        continue;
      }
      lane *= j;

      float y_min = canvas_min.y + lane * lane_h_ + 2;
      float y_max = y_min + lane_h_ - 4;

      const auto start_point = ImVec2(x_min, y_min);
      const auto end_point = ImVec2(x_max, y_max);

      if (ImGui::IsWindowHovered() &&
          RectContains(mouse, start_point, end_point))
        hovered = i;

      dl->AddRectFilled(start_point, end_point, lane_cols[lane], 2.0f);
      dl->AddRect(start_point, end_point, borderCol, 2.0f, 0, thick);
    }
  }

  static int last_hovered;
  static ImVec2 last_mouse;
  // --- pan/zoom when this child is hovered (updates shared timeline) ---
  if (ImGui::IsWindowHovered()) {
    ImGuiIO& io = ImGui::GetIO();

    /*

    // pan with wheel
    if (!io.KeyCtrl && io.MouseWheel != 0.0f) {
        ctx.timing.scroll_qn -= io.MouseWheel * 0.5; // beats per notch
    }
    // pan with middle-drag
    if (ImGui::IsMouseDown(ImGuiMouseButton_Middle)) {
        ctx.timing.scroll_qn -= io.MouseDelta.x / ctx.timing.pxPerQn();
    }

    */
    // zoom with Ctrl+wheel (anchor at mouse X)
    if (io.KeyCtrl && io.MouseWheel != 0.0f) {
      double mouse_qn = ctx_.timing.XToTick(io.MousePos.x, canvas_min.x) /
                        double(ctx_.timing.tpq);
      ctx_.timing.ZoomAbout(mouse_qn, io.MouseWheel > 0.0f ? 1.1 : 1.0 / 1.1);
    }

    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
      const bool ctrl = io.KeyCtrl;
      const bool shift = io.KeyShift;

      if (hovered < 0) {
        ctx_.sel_man.Clear();  // clicked empty area
      } else {
        const auto& n = notes[hovered];
        auto toggle_note = [&](const auto& note) {
          core::chart::NoteKey k{ctx_.difficulty, ctx_.instrument, note.id};
          ctx_.sel_man.Toggle(k);
        };

        if (ctrl && !shift) {
          // Ctrl-only: toggle hovered
          toggle_note(n);
        } else if (!ctrl && shift) {
          // Shift-only: select range from anchor to hovered
          SelectRange(n.on);
        } else {
          // Default (includes Ctrl+Shift or neither): single select
          ctx_.sel_man.Clear();
          toggle_note(n);
        }
      }
    }

    if (ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
      last_hovered = hovered;
      last_mouse = io.MousePos;
      ImGui::OpenPopup("TrackContext");
    }
  }

  if (ImGui::BeginPopup("TrackContext")) {
    if (ImGui::MenuItem("Add Note", 0, false, last_hovered < 0)) {
      int lane = 1 << int((last_mouse.y - canvas_min.y) / lane_h_);
      assert(lane >= 0 && lane <= 15 && Count4Bit(lane) == 1);
      int tick = ctx_.timing.XToTick(last_mouse.x, canvas_min.x);
      NoteIR new_note;
      new_note.origin = NoteOrigin::UserAdded;
      new_note.on = tick;
      new_note.off = tick + 120;
      new_note.velocity = 60;
      new_note.lane = lane;
      new_note.id = next_id++;

      auto& ir_notes = ctx_.chart_doc.working_chart.diff_tracks[ctx_.difficulty]
                           .tracks[ctx_.instrument]
                           .notes;
      auto it = std::lower_bound(
          ir_notes.begin(), ir_notes.end(), tick,
          [](const NoteIR& n, int on_tick) { return n.on < on_tick; });

      ir_notes.insert(it, new_note);

      ctx_.chart_doc.DirtyInstrumentCache(ctx_.difficulty, ctx_.instrument);
      ImGui::CloseCurrentPopup();
    }

    if (ImGui::MenuItem("Delete Selected", 0, false, !ctx_.sel_man.IsEmpty())) {
      std::unordered_map<int, std::vector<int>> byInst;

      ctx_.sel_man.ForEach([&](const core::chart::NoteKey& k) {
        if (k.difficulty == ctx_.difficulty) {
          byInst[k.instrument].push_back(k.id);
        }
      });

      for (int i = 0; i < 4; ++i) {
        auto& ir_notes =
            ctx_.chart_doc.working_chart.diff_tracks[ctx_.difficulty]
                .tracks[i]
                .notes;
        for (const auto id : byInst[i]) {
          // First *a* note in the corresponding tick/note group
          auto note_it =
              std::find_if(ir_notes.begin(), ir_notes.end(),
                           [id](const NoteIR& n) { return n.id == id; });

          ir_notes.erase(note_it);
        }
      }

      ctx_.sel_man.Clear();  // clear after deletion
      ctx_.chart_doc.DirtyTrackDiffCache(ctx_.difficulty);
      ImGui::CloseCurrentPopup();
    }

    ImGui::EndPopup();
  }

  ImGui::EndChild();
  ImGui::PopStyleVar();
  ImGui::PopID();
}

void TrackView::SelectRange(int selected_tick) const {
  uint32_t min_sel_tick = 0xFFFFFFFF;
  const auto& ir_notes =
      ctx_.chart_doc.working_chart.diff_tracks[ctx_.difficulty]
          .tracks[ctx_.instrument]
          .notes;

  if (ctx_.sel_man.IsEmpty()) {
    return;
  }

  // Find the selected note with the smallest tick
  ctx_.sel_man.ForEach([&](const core::chart::NoteKey& key) {
    if (key.difficulty == ctx_.difficulty &&
        key.instrument == ctx_.instrument) {
      auto ir_note =
          std::find_if(ir_notes.begin(), ir_notes.end(),
                       [&](const NoteIR& n) { return n.id == key.id; });
      min_sel_tick = std::min(static_cast<uint32_t>(ir_note->on), min_sel_tick);
    }
  });

  // We have no selected notes for this difficulty and instrument
  if (min_sel_tick == 0xFFFFFFFF) {
    return;
  }

  for (const auto& note : ir_notes) {
    if (selected_tick >= min_sel_tick) {
      // Select notes in a forward manner
      if (note.on >= min_sel_tick && note.on <= selected_tick) {
        ctx_.sel_man.Add(
            {ctx_.difficulty, ctx_.instrument, static_cast<int>(note.id)});
      }
    } else {
      // Select notes in a backward manner
      if (note.on >= selected_tick && note.on <= min_sel_tick) {
        ctx_.sel_man.Add(
            {ctx_.difficulty, ctx_.instrument, static_cast<int>(note.id)});
      }
    }
  }
}

const std::vector<core::chart::ViewNote>& TrackView::GetOrBuildCache(
    int diff, int track) {
  auto& tc = ctx_.chart_doc.track_cache[diff][track];

  if (tc.dirty) {
    PostPipeline post_pipe;
    tc.cached_view =
        post_pipe.BuildView(ctx_.chart_doc, diff, track, ctx_.post_conf);
    tc.dirty = false;
  }

  return tc.cached_view;
}

}  // namespace gui