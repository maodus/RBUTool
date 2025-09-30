#include "core/post/Passes.hpp"

#include <util/BitUtils.h>

#include <algorithm>
#include <iostream>
#include <unordered_set>

static bool IsEligible(const NoteIR* note) {
  if (!note) {
    return false;
  }

  return (!(note->modified) && note->origin == NoteOrigin::Imported);
}

std::vector<core::chart::ViewNote> GemPruner::Run(
    const PostContext& ctx, std::vector<core::chart::ViewNote>& notes) const {
  if (!ctx.cfg.do_pruning) {
    return notes;
  }

  const auto& tempo_map = *ctx.chart.working_chart.tempo_map;
  const int ms_window = static_cast<int>(ctx.cfg.prune_window);  // e.g., 120
  std::vector<core::chart::ViewNote> out;
  out.reserve(notes.size());

  int r = 1;
  int l = 0;

  auto Eligible = [&](const core::chart::ViewNote& v) {
    auto it = ctx.note_map.find(v.id);
    return it != ctx.note_map.end() && IsEligible(it->second);
  };

  std::unordered_set<int> keep_ids;
  keep_ids.reserve(notes.size());

  auto PushNote = [&](const core::chart::ViewNote& v) {
    if (keep_ids.find(v.id) == keep_ids.end()) {
      out.push_back(v);
      keep_ids.insert(v.id);
    }
  };

  std::cout << "Size of note_stream: " << notes.size() << std::endl;

  while (r < notes.size()) {
    const auto& l_note = notes[l];
    const auto& r_note = notes[r];

    const bool l_elig = Eligible(l_note);
    const bool r_elig = Eligible(r_note);

    // If both notes are ineligable for pruning, just emit both gems
    // We dont consider non eligible gems for pruning.
    if (!l_elig && !r_elig) {
      PushNote(l_note);
      PushNote(r_note);
      l = r + 1;
      r = l + 1;
      continue;
    }

    // If the left is in-eligible, push the left note and advance to next set
    if (!l_elig) {
      PushNote(l_note);
      ++l;
      ++r;
      continue;
    }

    // If the right is in-eligible, stick to the left but expand window forwards
    // by one
    if (!r_elig) {
      PushNote(r_note);
      ++r;
      continue;
    }

    const double diff_ms =
        tempo_map.TickToMs(r_note.on) - tempo_map.TickToMs(l_note.on);
    if (diff_ms <= ms_window) {
      PushNote(l_note);
      r++;
    } else {
      PushNote(l_note);
      PushNote(r_note);
      l = r;
      r = l + 1;
    }
  }
  std::cout << "Size of note_stream after pruning: " << out.size() << std::endl;
  return out;
}

static int bit_lookup[16] = {0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4};

std::vector<core::chart::ViewNote> SparsifyMultiGemPass::Run(
    const PostContext& ctx, std::vector<core::chart::ViewNote>& notes) const {
  if (!ctx.cfg.do_sparse_mgem) {
    return notes;
  }

  std::vector<core::chart::ViewNote> out = notes;

  for (auto& note : out) {
    const auto lane = note.lane;
    const auto note_ir = ctx.note_map.find(note.id)->second;

    if (IsEligible(note_ir) && bit_lookup[lane] == 2) {
      const uint8_t l_combo = (1 << 0) | (1 << 1);
      const uint8_t r_combo = (1 << 2) | (1 << 3);
      if (!(lane ^ l_combo) || !(lane ^ r_combo)) {
        note.lane ^= ((1 << 0) | (1 << 3));
      }
    }
  }

  return out;
}

static constexpr uint8_t reduction_map[16] = {0, 1, 2, 1, 4, 4, 2, 4,
                                              8, 8, 2, 8, 4, 8, 2, 1};

std::vector<core::chart::ViewNote> SeperateMultiGemPass::Run(
    const PostContext& ctx, std::vector<core::chart::ViewNote>& notes) const {
  if (!ctx.cfg.do_mgem_reduction) {
    return notes;
  }

  const uint32_t num_notes = notes.size();
  uint32_t note_start = 0;

  std::vector<core::chart::ViewNote> out = notes;

  while (note_start < num_notes) {
    uint32_t end_id = std::min(num_notes, note_start + 4);
    bool reduce_chain = true;

    for (uint32_t i = note_start + 1; i < end_id; ++i) {
      const auto& prev_note = notes[i - 1];
      const auto& cur_note = notes[i];

      const bool is_multi_chain =
          (Count4Bit(prev_note.lane) >= 2 && Count4Bit(cur_note.lane) >= 2);

      if (!is_multi_chain) {
        reduce_chain = false;
        break;
      }
    }

    if (!reduce_chain || end_id - note_start != 4) {
      note_start += 1;
    } else {
      for (uint32_t i = note_start; i < end_id; ++i) {
        out[i].lane = reduction_map[out[i].lane];
      }
      note_start = end_id;
    }
  }
  return out;
}
