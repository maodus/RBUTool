#include "core/midi/MidiProcessor.hpp"

#include <iostream>

namespace core::midi {
using namespace smf;
/* Rework this, it smells */
static MappingBehaviour behaviors[2] = {
    {// First MappingBehaviour
     {{
         {{61, 73, 85, 97}},  // row 0
         {{60, 72, 84, 96}},  // row 1
         {{60, 72, 84, 96}},  // row 2
         {{60, 72, 84, 96}}   // row 3
     }},
     {{3, 3, 3, 3}}},
    {// Second MappingBehaviour (pitched vocals)
     {{
         {{61, 73, 85, 97}},  // row 0
         {{60, 72, 84, 96}},  // row 1
         {{60, 72, 84, 96}},  // row 2
         {{36, 36, 36, 36}}   // row 3
     }},
     {{3, 3, 3, 47}}}};

bool MidiProcessor::LoadMidi(const std::string& filepath) {
  if (!_midifile.read(filepath)) {
    return false;
  }
  _midifile.linkNotePairs();

  std::cout << "TPQ: " << _midifile.getTicksPerQuarterNote() << std::endl;
  std::cout << "TRACKS: " << _midifile.getTrackCount() << std::endl;

  tempo_map = std::make_shared<TempoMap>(_midifile.getTicksPerQuarterNote());
  measure_map = std::make_shared<MeasureMap>();
  beat_map = std::make_shared<BeatMap>();

  for (int i = 0; i < _midifile[0].size(); ++i) {
    MidiEvent& event = _midifile[0][i];
    if (event.isTempo()) {
      tempo_map->AddTempo(event.tick, event.getTempoMicroseconds());
    } else if (event.isTimeSignature()) {
      const auto mbt = measure_map->GetMBT(event.tick);
      const int num = static_cast<int>(event[3]);
      const int den = 1 << static_cast<int>(event[4]);
      measure_map->AddTimeSig(mbt.measure, num, den);
      std::cout << mbt.measure << std::endl;
    }
  }

  for (int i = 0; i < _midifile.size(); ++i) {
    const auto& track = _midifile[i];
    const int num_events = track.size();

    // The track has no events, cant do much
    if (num_events < 1) {
      continue;
    }

    const MidiEvent& first_event = track[0];
    if (!first_event.isMeta() || first_event.getMetaType() != 3 ||
        first_event.getMetaContent() != "BEAT") {
      continue;
    }

    for (int j = 0; j < track.size(); ++j) {
      const auto& event = track[j];
      if (event.isNoteOn()) {
        beat_map->AddBeat(event.tick, event.getKeyNumber());
      }
    }
  }
  return true;
}

static std::vector<std::string> parts{"PART DRUMS", "PART BASS", "PART GUITAR",
                                      "PART VOCALS"};

static int difficulties[4][4] = {
    {61, 73, 85, 97}, {60, 72, 84, 96}, {60, 72, 84, 96}, {60, 72, 84, 96}};

MidiProcessor::MidiProcessor(bool is_pitched)
    : core::chart::IChartImporter(is_pitched) {}

bool MidiProcessor::ImportChart(const std::string& file_path) {
  std::cout << file_path << " " << is_pitched << std::endl;
  LoadMidi(file_path);
  return false;
}

void MidiProcessor::ToGuiTrack(ChartIR& chart) {
  uint32_t last_tick = 0;

  chart.beat_map = this->beat_map;
  chart.tempo_map = this->tempo_map;
  chart.measure_map = this->measure_map;
  int id = 0;

  for (int i = 0; i < _midifile.getTrackCount(); ++i) {
    MidiEventList& mid_track = _midifile[i];
    const int num_events = mid_track.size();

    // The track has no events, skip the track
    if (num_events < 1) {
      continue;
    }

    // Our first event should be the track name
    MidiEvent& first_event = mid_track[0];
    if (!first_event.isMeta() || first_event.getMetaType() != 3 ||
        std::find(parts.begin(), parts.end(), first_event.getMetaContent()) ==
            parts.end()) {
      continue;
    }

    const std::string track_name = first_event.getMetaContent();
    int track_idx = 0;
    for (int j = 0; j < 4; j++) {
      if (track_name == parts[j]) {
        track_idx = j;
      }
    }

    last_tick = std::max((int)last_tick, mid_track.back().getTickDuration() +
                                             mid_track.back().tick);

    for (int j = 0; j < num_events; ++j) {
      MidiEvent& event = mid_track[j];

      if (event.isNoteOn() && event.isLinked()) {
        int key = event.getKeyNumber();

        // Create note base
        NoteIR note;
        note.id = id++;
        note.pitch = static_cast<char>(key);
        note.on = event.tick;
        note.off = event.tick + event.getTickDuration();
        note.modified = false;
        note.origin = NoteOrigin::Imported;

        // If its an event, handle that seperately and continue onto next note
        EventKind event_type = GetEventType(note);
        if (event_type != EventKind::Unknown) {
          TrackEvent track_event;
          track_event.on = note.on;
          track_event.off = note.off;
          track_event.type = event_type;
          chart.track_events[track_idx].events.push_back(track_event);
          continue;
        }

        // If we recognize the note, proceed, else, just keep track of it for
        // now. It will need to be re-exported once exports are implemented.
        int diff = GetNoteDifficulty(note, track_idx);
        if (diff < 0) {
          chart.unk_notes->notes.push_back(note);
          continue;
        }

        int k = (is_pitched && track_idx == 3) ? 0 : diff;
        const int diff_end = (is_pitched && track_idx == 3) ? 4 : diff + 1;

        for (; k < diff_end; ++k) {
          const int diff_start = static_cast<int>(
              behaviors[static_cast<int>(is_pitched)].diff_map[track_idx][k]);
          note.lane = 1 << static_cast<char>((key - diff_start) % 4);

          chart.diff_tracks[k].tracks[track_idx].notes.push_back(note);
        }
      }
    }
  }

  std::cout << id << std::endl;
  chart.final_tick = last_tick;
}

int MidiProcessor::GetNoteDifficulty(const NoteIR& note, int part) const {
  int diff = -1;

  for (int i = 0; i < 4; ++i) {
    int diff_start = behaviors[this->is_pitched].diff_map[part][i];
    int diff_end = diff_start + behaviors[this->is_pitched].diff_span[part];

    if (note.pitch >= diff_start && note.pitch <= diff_end) {
      diff = i;
    }
  }

  return diff;
}

bool MidiProcessor::IsEvent(const NoteIR& note) const {
  if (note.pitch == 103 || note.pitch == 116) {
    return true;
  }
  return false;
}

EventKind MidiProcessor::GetEventType(const NoteIR& note) const {
  switch (note.pitch) {
    case 103:
      return EventKind::Solo;
    case 116:
      return EventKind::Overdrive;
    default:
      return EventKind::Unknown;
  }
}
}  // namespace core::midi
