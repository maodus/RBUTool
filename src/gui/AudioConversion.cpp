#include "gui/AudioConversion.hpp"

#include <core/Common.h>
#include <util/BackgroundTask.h>

#include <core/atm/ATMContainer.hpp>
#include <filesystem>
#include <gui/GuiHelpers.hpp>
#include <iostream>
#include <tool/AT3ToolRunner.hpp>
#include <tool/FFmpegRunner.hpp>

#include "ImGuiFileDialog.h"
#include "imgui.h"

// namespace fs = std::filesystem;

static std::string ToLower(std::string s) {
  for (auto& c : s)
    c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
  return s;
}
static std::string LowerExt(const fs::path& p) {
  return ToLower(p.extension().string());
}

// RAII temp dir remover
struct TempDir {
  fs::path path;
  bool keep = false;
  ~TempDir() {
    if (!keep && !path.empty()) {
      std::error_code ec;
      fs::remove_all(path, ec);
    }
  }
};

// Threaded job handle
static BackgroundTask g_task;

static void StartEncryptJob(const std::vector<GroupPick> in,
                            const fs::path& outAtm) {
  g_task.Start([=](ProgressReporter rep) {
    FFmpegRunner ffmpeg("ffmpeg.exe");
    AT3ToolRunner at3("at3tool.exe");

    // Prepare a unique temp directory next to the exe or cwd
    fs::path base = fs::current_path();
    fs::path temp = base / ("temp_atm_");
    std::error_code ec;
    fs::create_directories(temp, ec);

    TempDir temp_guard{temp, /*keep*/ false};

    // .at3 paths to feed encoder (some may be originals, some temp)
    std::array<std::string, 5> at3_paths{};

    rep.append_log("Starting conversion pipeline...\n");

    if (outAtm.empty()) {
      rep.append_log("WARN: output is empty; cancelling job");
      return;
    }

    for (int i = 0; i < 5; ++i) {
      if (rep.is_cancelled()) {
        rep.append_log("Cancelled by user.\n");
        return;
      }

      const GroupPick& g = in[i];
      const bool needs_mix = g.HasParts();
      std::vector<fs::path> parts = g.PartPaths();
      fs::path main = g.main.path;

      const std::string stem =
          needs_mix ? ("mix_" + std::to_string(i))
                    : (std::to_string(i) + "_" + main.stem().string());

      const fs::path wav_dst = temp / (stem + "_mono44100.wav");
      const fs::path at3_dst = temp / (stem + ".at3");

      if (needs_mix) {
        rep.append_log("Track " + std::to_string(i) + ": mixing " +
                       std::to_string(parts.size()) + " inputs...\n");
        std::vector<std::string> path_strings;
        path_strings.reserve(parts.size());
        std::transform(
            parts.begin(), parts.end(), std::back_inserter(path_strings),
            [](const std::filesystem::path& p) { return p.string(); });
        auto f = ffmpeg.MixToWav(path_strings, wav_dst.string());

        if (f.exit_code != 0) {
          rep.append_log(f.log_out);
          rep.append_log("  ERROR: MixToWav failed\n");
          return;
        }
        rep.append_log("  Encoding mixed WAV to AT3...\n");
        auto a = at3.WavToAt3(wav_dst.string(), at3_dst.string());

        if (a.exit_code != 0) {
          rep.append_log(f.log_out);
          rep.append_log("  ERROR: at3tool failed for mixed WAV\n");
          return;
        }
        at3_paths[i] = at3_dst.string();
        rep.set_progress((i + 1) * 0.20f);
      } else {
        const auto ext = LowerExt(main);
        fs::path wav_src = main;

        // Case 1: already .at3 -> use as-is
        if (ext == ".at3") {
          rep.append_log("  Using .at3 as-is.\n");
          at3_paths[i] = main.string();
          rep.set_progress((i + 1) * 0.20f);
          continue;
        }

        // Case 2: if not WAV -> ffmpeg -> mono 44.1kHz 16-bit PCM WAV (what
        // at3tool wants)
        if (ext != ".wav") {
          rep.append_log("  Transcoding to WAV (mono, 44.1kHz, 16-bit)...\n");
          auto f = ffmpeg.AudioToWav(main.string(), wav_dst.string());
          rep.append_log(f.log_out);
          if (f.exit_code != 0) {
            rep.append_log("  ERROR: ffmpeg failed for " + main.string() +
                           "\n");
            rep.append_log("  Please make sure ffmpeg is added to PATH.");
            return;
          }
          wav_src = wav_dst;
        } else {
          rep.append_log("  Input is WAV; expecting mono/44.1kHz/16-bit.\n");
        }

        if (rep.is_cancelled()) return;

        // Case 3: at3tool encode WAV -> AT3
        rep.append_log("  Encoding WAV to AT3...\n");
        auto a = at3.WavToAt3(wav_src.string(), at3_dst.string());
        rep.append_log(a.log_out);
        if (a.exit_code != 0) {
          rep.append_log("  ERROR: at3tool failed for " + wav_src.string() +
                         "\n");
          rep.append_log("  Please make sure at3tool is added to PATH.");
          return;
        }

        at3_paths[i] = at3_dst.string();

        // Update progress (5 tracks -> 20% each)
        rep.set_progress((i + 1) * 0.20f);
      }
    }

    if (rep.is_cancelled()) {
      rep.append_log("Cancelled before assembling ATM.\n");
      return;
    }

    // Assemble & encrypt into ATM container
    rep.append_log("Assembling & encrypting ATM...\n");
    {
      ATMWriter writer;
      writer.Write(at3_paths, outAtm.string().c_str());
    }
    rep.append_log("Finished writing: " + outAtm.string() + "\n");

    // All good
    rep.set_progress(1.0f);
    rep.append_log("Done.\n");

    // TempDir will auto-clean on scope exit
  });
}

AudioConversion::AudioConversion() {
  for (int i = 0; i < 5; ++i) {
    enc_ui.track_files.emplace_back(AudioTrackNames[i]);
  }

  enc_ui.output_file.main.label = "Output ATM";

  // https://github.com/TheNathannator/GuitarGame_ChartFormats/blob/main/doc/FileFormats/Audio%20Files.md
  enc_ui.track_files[0].AddNumberedParts("drums", 4);
  enc_ui.track_files[3].AddNumberedParts("vocals", 2);
}

void AudioConversion::Render() {
  if (ImGui::CollapsingHeader("Encrypt ATM")) {
    this->RenderEncryption();
  }

  if (ImGui::CollapsingHeader("Decrypt ATM")) {
    ImGui::SeparatorText("Soon");
  }
}

// one-liner file row
static void FileRow(const char* label, gui::TextBuf& buf) {
  ImGui::TableNextRow();
  ImGui::TableSetColumnIndex(0);
  ImGui::TextUnformatted(label);
  ImGui::TableSetColumnIndex(1);
  ImGui::SetNextItemWidth(-1);
  ImGui::InputText("##StemPath", buf.data.data(), buf.data.size(),
                   ImGuiInputTextFlags_ElideLeft);
}

// section for a grouped instrument (fallback + parts)
static void RenderGroup(const char* title, AudioFilePick& pick) {
  ImGui::SeparatorText(title);
  if (ImGui::BeginTable(
          title, 2,
          ImGuiTableFlags_Borders | ImGuiTableFlags_SizingStretchProp)) {
    ImGui::TableSetupColumn("Stem");
    ImGui::TableSetupColumn("Path");
    ImGui::TableHeadersRow();

    for (int i = 0; i < pick.parts.size(); ++i) {
      auto& p = pick.parts[i];
      ImGui::PushID(i);
      FileRow(p.label.c_str(), p.input);
      ImGui::PopID();
    }

    ImGui::EndTable();
  }
}

void AudioConversion::RenderAutoFill() {
  if (ImGui::Button("Choose Auto-fill Folder")) {
    IGFD::FileDialogConfig config;
    config.path = enc_ui.last_dir;
    config.flags =
        ImGuiFileDialogFlags_Modal | ImGuiFileDialogFlags_DontShowHiddenFiles;
    ImGui::SetNextWindowSize({560, 360}, ImGuiCond_Appearing);
    ImGuiFileDialog::Instance()->OpenDialog("AutoFillDir", "Choose a Directory",
                                            nullptr, config);
  }

  if (ImGuiFileDialog::Instance()->Display("AutoFillDir")) {
    std::string file_path = ImGuiFileDialog::Instance()->GetCurrentPath();

    if (ImGuiFileDialog::Instance()->IsOk()) {
      AutoFillFromDir(file_path);
    }
    enc_ui.last_dir = file_path;
    ImGuiFileDialog::Instance()->Close();
  }

  ImGui::Separator();
}

void AudioConversion::RenderMainFilePick(AudioFilePick& file_pick,
                                         ImGuiFileDialogFlags flags) {
  auto& main_pick = file_pick.main;

  ImGui::BeginDisabled(file_pick.HasActiveParts());

  // Render the path/text input
  ImGui::SetNextItemWidth(300.0f);
  ImGui::InputText("##TrackFilePath", main_pick.input.data.data(),
                   main_pick.input.data.size(), ImGuiInputTextFlags_ElideLeft);

  ImGui::SameLine();

  const auto dialog_id = "AudioPick##" + main_pick.label;

  // Render the select file button
  if (ImGui::Button("..##TrackFileDialog")) {
    IGFD::FileDialogConfig config;
    config.path = enc_ui.last_dir;
    config.flags = flags;
    ImGui::SetNextWindowSize({480, 320}, ImGuiCond_Appearing);
    ImGuiFileDialog::Instance()->OpenDialog(dialog_id, "Choose File", ".*",
                                            config);
  }

  // Render label
  ImGui::SameLine();
  ImGui::TextUnformatted(main_pick.label.c_str());

  ImGui::EndDisabled();

  if (ImGuiFileDialog::Instance()->Display(dialog_id)) {
    std::string file_path = ImGuiFileDialog::Instance()->GetCurrentPath();

    if (ImGuiFileDialog::Instance()->IsOk()) {
      std::string file_name = ImGuiFileDialog::Instance()->GetFilePathName();
      main_pick.input.Set(file_name);
    }
    enc_ui.last_dir = file_path;
    ImGuiFileDialog::Instance()->Close();
  }
}

void AudioConversion::RenderPartFilePick(AudioFilePick& file_pick) {
  RenderGroup(file_pick.main.label.c_str(), file_pick);
}

void AudioConversion::RenderOutput() {
  ImGui::SeparatorText("Output");
  ImGui::PushID("##EncOut");
  RenderMainFilePick(
      enc_ui.output_file,
      ImGuiFileDialogFlags_Modal | ImGuiFileDialogFlags_ConfirmOverwrite);
  ImGui::PopID();

  if (ImGui::Button("Encrypt")) {
    ImGui::OpenPopup("Encrypting...");
    StartEncryptJob(CollectInputs(), enc_ui.output_file.main.input.ToString());
  }
}

void AudioConversion::RenderEncryption() {
  ImGui::SeparatorText("Input");
  ImGui::Spacing();

  RenderAutoFill();

  const int track_count = static_cast<int>(AudioTrack::Count);

  // Render main file selector ui for audio tracks
  for (int i = 0; i < track_count; ++i) {
    auto& file_pick = enc_ui.track_files[i];
    ImGui::PushID(i);
    RenderMainFilePick(file_pick, ImGuiFileDialogFlags_Modal);
    ImGui::PopID();
  }

  // Render stem selection ui
  if (ImGui::TreeNode("Advanced")) {
    RenderPartFilePick(enc_ui.track_files[0]);
    RenderPartFilePick(enc_ui.track_files[3]);
    ImGui::TreePop();
  }

  RenderOutput();

  // Modal while running
  if (ImGui::BeginPopupModal("Encrypting...", nullptr,
                             ImGuiWindowFlags_AlwaysAutoResize |
                                 ImGuiWindowFlags_NoSavedSettings)) {
    ImGui::TextUnformatted("Converting & Encrypting");
    ImGui::Spacing();

    ImGui::ProgressBar(g_task.Progress(), ImVec2(400.0f, 0));
    ImGui::Spacing();

    // Live log
    ImGui::Separator();
    ImGui::BeginChild("log", ImVec2(500, 200), true,
                      ImGuiWindowFlags_NoSavedSettings);
    const auto log = g_task.LogSnapshot();
    ImGui::TextUnformatted(log.c_str());
    ImGui::EndChild();

    ImGui::Spacing();
    if (!g_task.Completed()) {
      if (ImGui::Button("Cancel")) {
        g_task.Cancel();
      }
    } else {
      if (ImGui::Button("Close")) {
        ImGui::CloseCurrentPopup();
      }
    }

    ImGui::EndPopup();
  }
}

void AudioConversion::RenderDecryption() {}

void AudioConversion::AutoFillFromDir(fs::path dir) {
  if (!fs::is_directory(dir)) {
    return;
  }

  std::unordered_map<std::string, std::vector<fs::path>> bucket;
  auto AddCandidate = [&](const std::string& key, const fs::path& p) {
    bucket[key].push_back(p);
  };

  for (auto& entry : fs::directory_iterator(dir)) {
    if (!entry.is_regular_file()) continue;
    const fs::path p = entry.path();
    const fs::path stem = p.stem();
    const fs::path ext = LowerExt(p);

    if (ext == ".ini") {
      continue;
    }

    if (stem == "drums") {
      AddCandidate("drums", p);
    } else if (stem == "drums_1") {
      AddCandidate("drums_1", p);
    } else if (stem == "drums_2") {
      AddCandidate("drums_2", p);
    } else if (stem == "drums_3") {
      AddCandidate("drums_3", p);
    } else if (stem == "drums_4") {
      AddCandidate("drums_4", p);
    } else if (stem == "bass") {
      AddCandidate("bass", p);
    } else if (stem == "rhythm") {
      AddCandidate("bass", p);
    } else if (stem == "guitar") {
      AddCandidate("guitar", p);
    } else if (stem == "vocals") {
      AddCandidate("vocals", p);
    } else if (stem == "vocals_1") {
      AddCandidate("vocals_1", p);
    } else if (stem == "vocals_2") {
      AddCandidate("vocals_2", p);
    } else if (stem == "song") {
      AddCandidate("song", p);
    }
  }

  auto ApplyPick = [&](const char* key, gui::TextBuf& target) {
    auto it = bucket.find(key);
    if (it == bucket.end() || it->second.empty() || !target.Empty()) {
      return false;
    }
    // TODO: Pick "best" file based on some sort of heuristic
    const fs::path& best = (it->second)[0];
    target.Set(best.string());
    return true;
  };

  auto& drums_pick = enc_ui.track_files[0];
  auto& drums_parts = drums_pick.parts;

  auto& vocals_pick = enc_ui.track_files[3];
  auto& vocals_parts = vocals_pick.parts;

  bool any_drums_parts = ApplyPick("drums_1", drums_parts[0].input) |
                         ApplyPick("drums_2", drums_parts[1].input) |
                         ApplyPick("drums_3", drums_parts[2].input) |
                         ApplyPick("drums_4", drums_parts[3].input);

  bool any_vocals_parts = ApplyPick("vocals_1", vocals_parts[0].input) |
                          ApplyPick("vocals_2", vocals_parts[1].input);

  if (!any_drums_parts) {
    ApplyPick("drums", drums_pick.main.input);
  }

  if (!any_vocals_parts) {
    ApplyPick("vocals", vocals_pick.main.input);
  }

  ApplyPick("bass", enc_ui.track_files[1].main.input);
  ApplyPick("guitar", enc_ui.track_files[2].main.input);
  ApplyPick("song", enc_ui.track_files[4].main.input);
}

std::vector<GroupPick> AudioConversion::CollectInputs() {
  std::vector<GroupPick> out;
  out.reserve(5);

  for (int i = 0; i < 5; ++i) {
    auto& pick = enc_ui.track_files[i];

    FilePick fp = {fs::path(pick.main.input.ToString())};
    GroupPick gp{};

    gp.main = fp;

    for (auto& part : pick.parts) {
      FilePick part_fp = {fs::path(part.input.ToString())};
      gp.parts.push_back(part_fp);
    }
    out.push_back(gp);
  }

  return out;
}
