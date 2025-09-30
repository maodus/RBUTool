#include <core/post/PostPipeline.hpp>
#include <iostream>

PostPipeline::PostPipeline() {
  passes_.push_back(std::make_unique<GemPruner>());
  passes_.push_back(std::make_unique<SparsifyMultiGemPass>());
  passes_.push_back(std::make_unique<SeperateMultiGemPass>());
}

std::vector<core::chart::ViewNote> PostPipeline::BuildView(
    const core::chart::ChartDocument& chart_doc, int diff, int track,
    const PostConfig& cfg) const {
  auto& ir_notes =
      chart_doc.working_chart.diff_tracks[diff].tracks[track].notes;

  std::unordered_map<int, const NoteIR*> note_map;
  note_map.reserve(ir_notes.size());

  std::vector<core::chart::ViewNote> note_stream;
  note_stream.reserve(ir_notes.size());

  for (const auto& note : ir_notes) {
    note_stream.emplace_back(
        core::chart::ViewNote{static_cast<int>(note.id), note.lane, note.on,
                              note.off, note.modifiers, note.velocity});
    note_map.try_emplace(note.id, &note);
  }

  PostContext ctx{chart_doc, static_cast<TrackDifficulty>(diff),
                  static_cast<GameTrack>(track), note_map, cfg};

  for (const auto& pass : passes_) {
    note_stream = pass->Run(ctx, note_stream);
  }

  return note_stream;
}
