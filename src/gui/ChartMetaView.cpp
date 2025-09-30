#include <core/Common.h>

#include <array>
#include <gui/ChartMetaView.hpp>
#include <gui/GuiHelpers.hpp>

ChartMetaView::ChartMetaView() {
  // std::memset(&rbu_header_, 0, sizeof(RBUHeader));
}

void ChartMetaView::Render() {
  static char in[2] = {'2', '\0'};

  if (ImGui::Begin("Chart_Metadata")) {
    ImGui::BeginDisabled(true);
    ImGui::InputText("Version", in, 2, ImGuiInputTextFlags_ReadOnly);
    ImGui::EndDisabled();

    ImGui::InputText("Song Title", rbu_header_->display_name, 48);
    ImGui::InputText("Artist", rbu_header_->artist_name, 48);

    CreateCombo("Difficulty", SongDiffNames, &rbu_header_->difficulty,
                ImGuiComboFlags_HeightRegular);
    CreateCombo("Genre", SongGenreNames, &rbu_header_->genre,
                ImGuiComboFlags_HeightLarge);
    CreateCombo("Initial Track", GameTrackNames, &rbu_header_->initial_track,
                ImGuiComboFlags_HeightSmall);

    ImGui::Spacing();
    ImGui::Spacing();

    ImGui::SliderInt("Drum Rating",
                     reinterpret_cast<int*>(&rbu_header_->drum_rating), 0, 6);
    ImGui::SliderInt("Bass Rating",
                     reinterpret_cast<int*>(&rbu_header_->bass_rating), 0, 6);
    ImGui::SliderInt("Guitar Rating",
                     reinterpret_cast<int*>(&rbu_header_->guitar_rating), 0, 6);
    ImGui::SliderInt("Vocal Rating",
                     reinterpret_cast<int*>(&rbu_header_->vocal_rating), 0, 6);
    ImGui::SliderInt("Band Rating",
                     reinterpret_cast<int*>(&rbu_header_->band_rating), 0, 6);
  }
  ImGui::End();
}