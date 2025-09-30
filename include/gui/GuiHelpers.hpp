#pragma once
#include <imgui.h>

#include <array>

inline void HelpMarker(const char* desc) {
  ImGui::TextDisabled("(?)");
  if (ImGui::BeginItemTooltip()) {
    ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
    ImGui::TextUnformatted(desc);
    ImGui::PopTextWrapPos();
    ImGui::EndTooltip();
  }
}

template <std::size_t N>
inline void CreateCombo(const char* label,
                        const std::array<const char*, N>& names, int* idx,
                        ImGuiComboFlags flags) {
  const char* combo_preview_value = names[*idx];
  if (ImGui::BeginCombo(label, combo_preview_value, flags)) {
    const int track_count = static_cast<int>(N);
    for (int n = 0; n < track_count; n++) {
      const bool is_selected = (*idx == n);
      if (ImGui::Selectable(names[n], is_selected)) {
        *idx = n;
      }

      // Set the initial focus when opening the combo (scrolling + keyboard
      // navigation focus)
      if (is_selected) ImGui::SetItemDefaultFocus();
    }
    ImGui::EndCombo();
  }
}