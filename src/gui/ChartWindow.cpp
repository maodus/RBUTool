#include "gui/ChartWindow.hpp"

#include <core/Common.h>
#include <util\BitUtils.h>

#include <core/RbuProcessor.hpp>
#include <core/post/PostPipeline.hpp>
#include <gui/GuiHelpers.hpp>
#include <iostream>
#include <unordered_map>

#include "core/midi/MidiProcessor.hpp"
#include "core/post/Passes.hpp"

void ChartWindow::RenderExport() {
  // Export modal
  if (ImGui::BeginPopupModal("Export Chart", NULL,
                             ImGuiWindowFlags_AlwaysAutoResize)) {
    // Manual filepath input with text
    ImGui::SetNextItemWidth(150.0f);
    ImGui::InputTextWithHint(
        "##TrackFilePath", "Path to chart file", this->fd_ctx.path_buf.data(),
        this->fd_ctx.path_buf.size(), ImGuiInputTextFlags_ElideLeft);

    ImGui::SameLine();
    if (ImGui::Button("..##OpenFile")) {
      IGFD::FileDialogConfig config;
      config.flags =
          ImGuiFileDialogFlags_Modal | ImGuiFileDialogFlags_ConfirmOverwrite;
      this->fd_ctx.dialog.OpenDialog("ExportChartDialog", "Export Chart",
                                     ".RBU", config);
    }

    if (this->fd_ctx.dialog.Display("ExportChartDialog")) {
      std::string cur_path = fd_ctx.dialog.GetCurrentPath();

      if (fd_ctx.dialog.IsOk()) {
        std::string file_path = fd_ctx.dialog.GetFilePathName();
        std::strncpy(fd_ctx.path_buf.data(), file_path.c_str(),
                     fd_ctx.path_buf.size() - 1);
        fd_ctx.path_buf.back() = '\0';
      }

      this->fd_ctx.dialog.Close();
    }
    static int exp_type = 0;
    ImGui::SameLine();
    CreateCombo("##ExportType", ExportTypes, &exp_type,
                ImGuiComboFlags_WidthFitPreview);

    ImGui::Separator();

    if (ImGui::Button("Ok##ExportDone")) {
      RBUProcessor rbup;

      rbup.WriteRBUFile(std::string(fd_ctx.path_buf.data()), this->cd,
                        this->rbu_header);

      ImGui::CloseCurrentPopup();
    }
    ImGui::SameLine();
    if (ImGui::Button("Cancel##ExportCancel")) {
      ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
  }
}

void ChartWindow::RenderImport() {  // Import pop-up modal
  if (ImGui::BeginPopupModal("Import a Chart", NULL,
                             ImGuiWindowFlags_AlwaysAutoResize)) {
    // Manual filepath input with text
    ImGui::SetNextItemWidth(150.0f);
    ImGui::InputTextWithHint(
        "##TrackFilePath", "Path to chart file", this->fd_ctx.path_buf.data(),
        this->fd_ctx.path_buf.size(), ImGuiInputTextFlags_ElideLeft);

    // Filepath selection with imgui browser
    ImGui::SameLine();
    if (ImGui::Button("..##OpenFile")) {
      IGFD::FileDialogConfig config;
      config.flags = ImGuiFileDialogFlags_Modal;
      this->fd_ctx.dialog.OpenDialog("ImportChartDialog", "Select Chart",
                                     "MIDI files{.mid,.midi}", config);
    }

    static bool is_pitched_vocals = false;
    if (this->fd_ctx.dialog.Display("ImportChartDialog")) {
      std::string cur_path = fd_ctx.dialog.GetCurrentPath();

      if (fd_ctx.dialog.IsOk()) {
        std::string file_path = fd_ctx.dialog.GetFilePathName();
        std::strncpy(fd_ctx.path_buf.data(), file_path.c_str(),
                     fd_ctx.path_buf.size() - 1);
        fd_ctx.path_buf.back() = '\0';
      }

      this->fd_ctx.dialog.Close();
    }

    ImGui::Checkbox("Pitched Vocals", &is_pitched_vocals);
    ImGui::SameLine();
    HelpMarker(
        "This setting is used if your vocal chart is not charted like "
        "a standard instrument, and instead uses vocal pitches. "
        "For example, a Fortnite Festival chart would have this set to "
        "false, "
        "whereas a standard Rock Band chart would have this set to true.");

    ImGui::Separator();
    if (ImGui::Button("Ok##ImportDone")) {
      this->chart_importer =
          std::make_unique<core::midi::MidiProcessor>(is_pitched_vocals);
      this->chart_importer->ImportChart(std::string(fd_ctx.path_buf.data()));
      this->chart_importer->ToGuiTrack(this->cd.working_chart);
      this->cd.DirtyAllTrackCache();

      this->UpdateBaseLine();
      this->fd_ctx.path_buf.fill('\0');
      ImGui::CloseCurrentPopup();
    }
    ImGui::SameLine();

    if (ImGui::Button("Cancel##ImportCancel")) {
      ImGui::CloseCurrentPopup();
    }

    ImGui::EndPopup();
  }
}

void ChartWindow::RenderNoneSelected() {
  if (ImGui::Begin(
          "ImportEmpty", 0,
          ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse)) {
    const auto& style = ImGui::GetStyle();
    const auto w_size = ImGui::GetWindowSize();
    ImGui::SetCursorPosX(w_size.x * 0.5f);
    ImGui::SetCursorPosY(w_size.y * 0.5f);

    if (ImGui::Button("Import")) {
      ImGui::OpenPopup("Import a Chart");
    }

    // Always center this window when appearing
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    RenderImport();
  }

  ImGui::End();
}

void ChartWindow::RenderChartSelected() {
  if (ImGui::Begin("B • Tracks")) {
    CreateCombo("Difficulty", TrackDiffNames, &this->difficulty,
                ImGuiComboFlags_HeightSmall);

    const float chart_height = lane_h * 16 + ImGui::GetStyle().ScrollbarSize +
                               (ImGui::GetStyle().ItemSpacing.y) * 3;

    // Compute content width from TPQ (no hard-coded 480)
    const double song_len_qn = this->cd.working_chart.final_tick / double(480);
    const float content_w =
        std::max(1.0f, float(song_len_qn * this->bt.PxPerQn()));
    float start_y = ImGui::GetCursorPosY();
    float cur_x = ImGui::GetCursorPosX();

    if (ImGui::BeginChild(
            "TrackNames",
            ImVec2(ImGui::GetStyle().ItemSpacing.x * 2, chart_height),
            ImGuiChildFlags_None,
            ImGuiWindowFlags_NoScrollbar |
                ImGuiWindowFlags_NoScrollWithMouse)) {
      for (int i = 0; i < 4; ++i) {
        float cur_y = ImGui::GetCursorPosY();
        int str_len = std::strlen(GameTrackNames[i]);
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, {0, 0});
        ImGui::SetCursorPosY(
            cur_y + (lane_h * 4 - ImGui::GetTextLineHeight() * str_len) * 0.5f);
        for (int j = 0; j < str_len; ++j) {
          ImGui::PushID(j);
          ImGui::Text("%c", GameTrackNames[i][j]);
          ImGui::PopID();
        }
        ImGui::PopStyleVar(1);
        ImGui::SetCursorPosY(cur_y + lane_h * 4 +
                             ImGui::GetStyle().ItemSpacing.y);
        ImGui::SetCursorPosX(0);
      }
      ImGui::Dummy(ImVec2(ImGui::GetStyle().ItemSpacing.x * 2, 480));

      ImGui::EndChild();
    }

    ImGui::SetCursorPosX(cur_x + ImGui::GetStyle().ItemSpacing.x * 2);
    ImGui::SetCursorPosY(start_y);

    ImGui::SetNextWindowContentSize(ImVec2(content_w, 0.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    // ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
    ImGui::BeginChild("mideee", ImVec2(0, chart_height), false,
                      ImGuiWindowFlags_HorizontalScrollbar);

    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImVec2 canvas_min = ImGui::GetCursorScreenPos();
    ImVec2 canvas_size = ImGui::GetContentRegionAvail();
    if (canvas_size.y < 60.0f) canvas_size.y = 60.0f;  // minimum height
    ImVec2 canvas_max =
        ImVec2(canvas_min.x + canvas_size.x, canvas_min.y + canvas_size.y);

    // Background
    dl->AddRectFilled(canvas_min, canvas_max,
                      ImGui::GetColorU32(ImGuiCol_FrameBg));

    ImU32 col_grid = ImGui::GetColorU32(ImVec4(1, 1, 1, 0.07f));
    ImU32 col_bar = ImGui::GetColorU32(ImVec4(1, 1, 1, 0.18f));
    ImU32 col_lane = ImGui::GetColorU32(ImVec4(1, 1, 1, 0.05f));

    for (int i = 0; i < 4; i++) {
      gui::TrackViewCtx tv_ctx{this->difficulty, i, cd, bt, sm, pc};
      gui::TrackView tv(22.0f, tv_ctx);
      tv.RenderTrack();
    }

    // Authoritative per-frame max scroll from ImGui
    const float max_px = ImGui::GetScrollMaxX();
    float cur_px = ImGui::GetScrollX();

    // Shift + wheel => horizontal scroll, even when hovering TrackView children
    ImGuiIO& io = ImGui::GetIO();
    if (ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows) &&
        io.MouseWheel != 0.0f && io.KeyShift) {
      cur_px -= io.MouseWheel * 40.0f;
      ImGui::SetScrollX(std::clamp(cur_px, 0.0f, max_px));
    }

    if (cur_px > max_px) {
      ImGui::SetScrollX(max_px);
    }

    bt.scroll_qn = double(ImGui::GetScrollX()) / bt.PxPerQn();

    ImGui::EndChild();
    ImGui::PopStyleVar();
  }

  RenderSelectedInfo();

  if (ImGui::CollapsingHeader("Post Processing")) {
    if (ImGui::Checkbox("Gem Pruning", &pc.do_pruning)) {
      cd.DirtyAllTrackCache();
    }
    ImGui::SameLine();

    ImGui::BeginDisabled(!pc.do_pruning);
    if (ImGui::SliderFloat("Prune Window", &pc.prune_window, 0, 300,
                           "%.2f ms")) {
      cd.DirtyAllTrackCache();
    }
    ImGui::EndDisabled();

    if (ImGui::Checkbox("Multi-Gem Reduction", &pc.do_mgem_reduction)) {
      cd.DirtyAllTrackCache();
    }

    if (ImGui::Checkbox("Sparse Multi-Gems", &pc.do_sparse_mgem)) {
      cd.DirtyAllTrackCache();
    }
  }

  if (ImGui::Button("Import New")) {
    ImGui::OpenPopup("Import a Chart");
  }

  ImGui::SameLine();

  // Button to open export modal
  if (ImGui::Button("Export##ExportModalButton")) {
    FinalizeTrackCaches();
    ImGui::OpenPopup("Export Chart");
  }

  RenderImport();
  RenderExport();
  ImGui::End();
}

void ChartWindow::RenderSelectedInfo() {
  if (sm.Count() != 1) return;

  const core::chart::NoteKey* nk = nullptr;

  sm.ForEach([&](const core::chart::NoteKey& note_key) { nk = &note_key; });
  auto& view_notes = cd.track_cache[nk->difficulty][nk->instrument].cached_view;
  auto view_note = std::find_if(
      view_notes.begin(), view_notes.end(),
      [&](const core::chart::ViewNote& vn) { return vn.id == nk->id; });
  auto& ir_notes =
      cd.working_chart.diff_tracks[nk->difficulty].tracks[nk->instrument].notes;
  auto ir_note = std::find_if(ir_notes.begin(), ir_notes.end(),
                              [&](const NoteIR& n) { return n.id == nk->id; });

  auto DrawSlideInput = [&](const char* slide_label, const char* input_label,
                            int* val, int min, int max) {
    ImGui::SetNextItemWidth(100.0f);
    const bool slide = ImGui::SliderInt(slide_label, val, min, max);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(100);
    const bool input = ImGui::InputInt(input_label, val);
    if (slide || input) {
      *val = std::clamp(*val, min, max);
      ir_note->modified = true;
      cd.track_cache[nk->difficulty][nk->instrument].dirty = true;
    }
  };

  ImGui::SeparatorText("Selected Note Information");

  DrawSlideInput("##TickOnSlide", "Tick Start##TickStartInput", &ir_note->on, 0,
                 ir_note->off);
  DrawSlideInput("##TickOffSlide", "Tick End##TickEndInput", &ir_note->off,
                 ir_note->on, cd.working_chart.final_tick);

  static bool lanes[4] = {};
  static const char* names[4] = {"Red", "Yellow", "Green", "Blue"};

  for (int i = 0; i < 4; ++i) {
    lanes[i] = static_cast<bool>(((view_note->lane) >> i) & 1);
    if (ImGui::Checkbox(names[i], &lanes[i])) {
      if ((Count4Bit(view_note->lane) > 1) | (lanes[i])) {
        ir_note->lane =
            view_note->lane;  // Update any discrepency between view and truth
        ir_note->lane ^= (1 << i);
        ir_note->modified = true;
        cd.track_cache[nk->difficulty][nk->instrument].dirty = true;
      }
    }

    if (i != 3) {
      ImGui::SameLine();
    }
  }

  const float ms_on = cd.working_chart.tempo_map->TickToMs(view_note->on);
  const float ms_off = cd.working_chart.tempo_map->TickToMs(view_note->off);

  ImGui::Separator();
  ImGui::Text("Start Ms %.2f", ms_on);
  ImGui::Text("End Ms %.2f", ms_off);
  ImGui::Text("Duration Ticks %d", view_note->off - view_note->on);
  ImGui::Text("Duration Ms %.2f", ms_off - ms_on);
  ImGui::Spacing();
}

void ChartWindow::CollapseNotes() {
  for (int i = 0; i < 4; ++i) {
    auto& ch = cd.working_chart.diff_tracks[i];
    for (int j = 0; j < 4; ++j) {
      NoteIR* last_note = nullptr;
      for (auto it = ch.tracks[j].notes.begin();
           it != ch.tracks[j].notes.end();) {
        if (last_note && it->on == last_note->on) {
          last_note->lane |= it->lane;
          last_note->pitch = std::min(it->pitch, last_note->pitch);
          it = ch.tracks[j].notes.erase(it);
        } else {
          last_note = &(*it);
          it++;
        }
      }
    }
  }
}

ChartWindow::ChartWindow() {}

void ChartWindow::render() {
  if (this->chart_importer == nullptr) {
    RenderNoneSelected();
  } else {
    RenderChartSelected();
  }
}

void ChartWindow::RenderTrack(int track_id) {
  const ImU32 col_lane = ImGui::GetColorU32(ImVec4(1, 1, 1, 0.05f));
  const ImU32 col_track = IM_COL32(255, 255, 255, 45);

  ImGui::PushID(track_id);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
  ImGui::BeginChild(
      "trackview", ImVec2(0, lane_h * 4), true,
      ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
  ImDrawList* dl = ImGui::GetWindowDrawList();
  ImVec2 canvas_min = ImGui::GetCursorScreenPos();
  ImVec2 canvas_size = ImGui::GetContentRegionAvail();
  ImVec2 canvas_max =
      ImVec2(canvas_min.x + canvas_size.x, canvas_min.y + canvas_size.y);

  for (int i = 1; i < 4; ++i) {
    float y = canvas_min.y + i * lane_h;
    dl->AddLine(ImVec2(canvas_min.x, y), ImVec2(canvas_max.x, y), col_lane,
                2.0f);
  }

  ImGui::EndChild();
  ImGui::PopStyleVar();
  ImGui::PopID();
}

void ChartWindow::UpdateBaseLine() {
  CollapseNotes();
  this->cd.base_chart = {this->cd.working_chart};
}

void ChartWindow::FinalizeTrackCaches() {
  // We dont check if its dirty here because we need all caches
  // built, otherwise notes on difficulties that have not been selected
  // will not be exported correctly.
  for (int k = 0; k < 4; ++k) {
    for (int l = 0; l < 4; ++l) {
      auto& tc = cd.track_cache[k][l];
      PostPipeline post_pipe;
      tc.cached_view = post_pipe.BuildView(cd, k, l, pc);
      tc.dirty = false;
    }
  }
}
