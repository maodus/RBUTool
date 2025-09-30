#pragma once

#include <MidiFile.h>

#include <core/Beat.hpp>
#include <core/ChartIR.hpp>
#include <core/Measure.hpp>
#include <core/Tempo.hpp>
#include <core/TrackEventManager.hpp>
#include <string>

#include "core/chart/ChartImporter.hpp"

namespace core::midi {
struct MappingBehaviour {
  std::array<std::array<uint8_t, 4>, 4> diff_map;
  std::array<uint32_t, 4> diff_span;
};

class MidiProcessor : public core::chart::IChartImporter {
 public:
  MidiProcessor(bool is_pitched);

  bool ImportChart(const std::string& file_path) override;
  void ToGuiTrack(ChartIR& chart) override;

  bool LoadMidi(const std::string& filepath);

 private:
  smf::MidiFile _midifile;
  std::shared_ptr<TempoMap> tempo_map;
  std::shared_ptr<MeasureMap> measure_map;
  std::shared_ptr<BeatMap> beat_map;

  int GetNoteDifficulty(const NoteIR& note, int part) const;
  bool IsEvent(const NoteIR& note) const;
  EventKind GetEventType(const NoteIR& note) const;
};
}  // namespace core::midi