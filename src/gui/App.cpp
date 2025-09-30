#include "gui/App.hpp"

#include <imgui_internal.h>

#include "imgui.h"

void App::DrawRootDockHost() {
  ImGuiWindowFlags f =
      ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar |
      ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
      ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus |
      ImGuiWindowFlags_NoNavFocus;

  const ImGuiViewport* vp = ImGui::GetMainViewport();
  ImGui::SetNextWindowPos(vp->Pos);
  ImGui::SetNextWindowSize(vp->Size);
  ImGui::SetNextWindowViewport(vp->ID);

  ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
  ImGui::Begin("RootDockHost", nullptr, f);
  ImGui::PopStyleVar(2);

  ImGuiID root_id = ImGui::GetID("RootDockSpace");

  // First run layout
  static bool built = false;
  if (!built) {
    built = true;
    ImGui::DockBuilderRemoveNode(root_id);
    ImGui::DockBuilderAddNode(root_id, ImGuiDockNodeFlags_DockSpace);
    ImGui::DockBuilderSetNodeSize(root_id, vp->Size);
    ImGui::DockBuilderDockWindow("Audio", root_id);
    ImGui::DockBuilderDockWindow("Chart", root_id);
    ImGui::DockBuilderFinish(root_id);
  }

  ImGui::DockSpace(root_id, ImVec2(0, 0), 0);

  ImGui::End();  // RootDockHost
}

void App::DrawAudioWindow() {
  if (ImGui::Begin("Audio", nullptr,
                   ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
                       ImGuiWindowFlags_NoMove)) {
    audio_window_->Render();
  }
  ImGui::End();
}

void App::DrawChartWindow() {
  static ImGuiWindowClass B_CLASS;

  static bool testee = false;

  static ImGuiID leftID = 0;
  static ImGuiID rightID = 0;
  static ImGuiID leftTopID = 0;
  static ImGuiID leftBottomID = 0;

  if (ImGui::Begin("Chart", nullptr,
                   ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
                       ImGuiWindowFlags_NoMove)) {
    ImGuiID chart_dock_id = ImGui::GetID("Chart_DockSpace");

    // First-run default layout inside B
    static bool built_b = false;
    if (!built_b) {
      built_b = true;

      ImVec2 r1 = ImGui::GetWindowContentRegionMin();
      ImVec2 r2 = ImGui::GetWindowContentRegionMax();

      ImGui::DockBuilderRemoveNode(chart_dock_id);
      ImGui::DockBuilderAddNode(chart_dock_id, ImGuiDockNodeFlags_DockSpace);
      ImGui::DockBuilderSetNodeSize(chart_dock_id,
                                    ImVec2(r2.x - r1.x, r2.y - r1.y));
      ImGui::DockBuilderSplitNode(chart_dock_id, ImGuiDir_Left, 0.3f, &leftID,
                                  &rightID);
      ImGui::DockBuilderFinish(chart_dock_id);
    }

    ImGui::DockSpace(chart_dock_id, ImVec2(0, 0), 0, &B_CLASS);
    B_CLASS.ClassId = chart_dock_id;
    B_CLASS.DockingAllowUnclassed = false;
    B_CLASS.DockNodeFlagsOverrideSet =
        ImGuiDockNodeFlags_NoTabBar | ImGuiDockNodeFlags_NoWindowMenuButton;

    // Draw the chart view on the rhs of the dockspace
    ImGui::SetNextWindowClass(&B_CLASS);
    ImGui::SetNextWindowDockID(rightID);
    chart_window_->render();

    // Draw the chart meta data view on the lhs of the dockspace
    ImGui::SetNextWindowClass(&B_CLASS);
    ImGui::SetNextWindowDockID(leftID);
    chart_meta_window_->Render();
  }

  ImGui::End();
}

App::App()
    : chart_meta_window_(std::make_unique<ChartMetaView>()),
      chart_window_(std::make_unique<ChartWindow>()),
      audio_window_(std::make_unique<AudioConversion>()) {
  auto rbu_header = std::make_shared<RBUHeader>();
  rbu_header->version = 2;
  chart_meta_window_->rbu_header_ = rbu_header;
  chart_window_->rbu_header = rbu_header;
}

void App::render() {
  DrawRootDockHost();
  DrawAudioWindow();
  DrawChartWindow();
}