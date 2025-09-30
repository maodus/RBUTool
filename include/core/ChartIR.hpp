#pragma once
#include <core/Beat.hpp>
#include <core/Measure.hpp>
#include <core/Tempo.hpp>
#include <cstdint>
#include <vector>

enum class EventKind { Unknown, Overdrive, Solo };

struct TrackEvent {
  int on;
  int off;
  EventKind type;
};

struct EventList {
  std::vector<TrackEvent> events;
};

enum class NoteOrigin : char { Imported, UserAdded };

// Instrument note/event facts (format-neutral)
struct NoteIR {
  long long id{0};
  char pitch{60};
  char lane{0};
  int on{0};
  int off{0};
  int modifiers{0};
  std::uint8_t velocity{100};
  std::uint8_t channel{0};
  NoteOrigin origin{NoteOrigin::Imported};
  bool modified;
};

// Each track has a list of notes
struct TrackIR {
  std::vector<NoteIR> notes;
};

struct TrackList {
  TrackIR tracks[4];
};

struct ChartIR {
  int resolution{480};
  TrackList diff_tracks[4];
  EventList track_events[4];
  TrackIR unk_notes[4];

  std::shared_ptr<BeatMap> beat_map;
  std::shared_ptr<TempoMap> tempo_map;
  std::shared_ptr<MeasureMap> measure_map;

  uint32_t final_tick;
};

struct ChartBaseLine {
  TrackList diff_tracks[4];
};