#include <core/BinaryIO.hpp>
#include <core/RbuProcessor.hpp>
#include <core/TrackEventManager.hpp>
#include <core\GameDefs.hpp>

static std::vector<GameGem> BuildGemsForTrack(
    const std::vector<core::chart::ViewNote> notes, const TempoMap& tempo) {
  std::vector<GameGem> gems;
  gems.reserve(notes.size());

  for (const auto& note : notes) {
    const int tick_on = note.on;
    const int tick_off = note.off;

    const float ms_on = tempo.TickToMs(tick_on);
    const float ms_off = tempo.TickToMs(tick_off);

    GameGem g{};
    g.ms = ms_on;
    g.tick = tick_on;
    g.duration_ms = static_cast<int16_t>(ms_off - ms_on);
    g.duration_tick = static_cast<int16_t>(tick_off - tick_on);
    g.unknown = 0;

    g.lane = note.lane;
    g.unknown2 = 0;

    g.unknown3 = 0x8;
    if ((tick_off - tick_on) <= 64 * 480 / 192) {
      g.unknown3 |= 2;
    }
    g.raw_flags = 0;

    gems.push_back(g);
  }

  return gems;
}

void RBUProcessor::WriteRBUFile(std::string filepath,
                                const core::chart::ChartDocument& chart,
                                std::shared_ptr<RBUHeader> header) {
  BinaryWriter bio(filepath);

  header->write(bio);
  chart.working_chart.tempo_map->write(bio);
  chart.working_chart.beat_map->write(bio);
  chart.working_chart.measure_map->write(bio);

  for (int j = 0; j < 4; ++j) {
    const auto diff_begin = bio.Tell();
    bio.U32(0);
    std::array<std::vector<GameGem>, 4> part_gems;

    // Pass 1: write note bytes for each part in order
    for (int part = 0; part < 4; ++part) {
      const auto& track = chart.track_cache[j][part].cached_view;

      auto gems = BuildGemsForTrack(track, *chart.working_chart.tempo_map);
      // Apply events / flags on gems
      TrackEventManager tem(chart.working_chart.track_events[part]);
      tem.ApplyEvents(gems, part);

      // persist for later passes
      part_gems[part] = std::move(gems);

      // Write: length then gem payloads (like Python “get_bytes”)
      bio.U32(static_cast<uint32_t>(part_gems[part].size()));
      for (const auto& g : part_gems[part]) {
        g.write(bio);
      }
    }

    // Pass 2: bars for each part
    for (int part = 0; part < 4; ++part) {
      // Determine last_tick for this difficulty/part (use last gem end tick if
      // you track it; otherwise a conservative choice is last gem's tick +
      // duration)
      uint32_t last_tick = 0;
      if (!part_gems[part].empty()) {
        const auto& last = part_gems[part].back();
        last_tick =
            static_cast<uint32_t>(last.tick) +
            static_cast<uint32_t>(std::max<int16_t>(0, last.duration_tick));
      }

      auto bars = BuildGemTrackBars(
          part_gems[part], chart.working_chart.tempo_map,
          chart.working_chart.measure_map, chart.working_chart.final_tick);

      bio.U32(static_cast<uint32_t>(bars.size()));
      for (const auto& b : bars) {
        b.write(bio);
      }
    }

    // Pass 3: solos for each part
    for (int part = 0; part < 4; ++part) {
      auto solos = BuildGemTrackSolos(part_gems[part]);
      bio.U32(static_cast<uint32_t>(solos.size()));
      for (const auto& s : solos) {
        s.write(bio);
      }
    }

    const auto diff_end = bio.Tell();
    bio.Seek(diff_begin);
    bio.U32(diff_end - diff_begin - 4);
    bio.Seek(diff_end);
  }
}

std::vector<SoloPhrase> RBUProcessor::BuildGemTrackSolos(
    std::vector<GameGem>& gems) const {
  std::vector<SoloPhrase> phrases;
  for (int i = 0; i < gems.size(); ++i) {
    const auto& cur_gem = gems[i];
    if (cur_gem.IsSoloStart()) {
      const int end_idx = cur_gem.GetCaptures() - 1 + i;
      const auto& end_gem = gems[end_idx];

      SoloPhrase phrase;
      phrase.ms = cur_gem.ms;
      phrase.duration_ms = (end_gem.duration_ms + end_gem.ms) - cur_gem.ms;
      phrase.tick = cur_gem.tick;
      phrase.duration_tick =
          (end_gem.duration_tick + end_gem.tick) - cur_gem.tick;
      phrase.gems_captured = cur_gem.GetCaptures();

      phrases.push_back(phrase);

      i = end_idx + 1;
    }
  }

  return phrases;
}

std::vector<BarInfo> RBUProcessor::BuildGemTrackBars(
    std::vector<GameGem>& gems, std::shared_ptr<TempoMap> tempo_map,
    std::shared_ptr<MeasureMap> measure_map, uint32_t last_tick) const {
  std::vector<BarInfo> bars;
  uint32_t cur_tick = 0;
  uint32_t gem_idx = 0;

  while (cur_tick < last_tick) {
    const auto t_sig = measure_map->GetTimeSig(cur_tick);
    const uint32_t bar_start = cur_tick;
    const uint32_t bar_end =
        bar_start + (480 * (4 / t_sig.denominator)) * t_sig.numerator;

    const uint32_t gem_start = gem_idx;
    for (; gem_idx < gems.size(); ++gem_idx) {
      const auto& gem = gems[gem_idx];
      if (gem.tick > bar_end) {
        break;
      }
    }

    BarInfo bar;
    bar.start_gem_id = gem_start;
    bar.end_gem_id = gem_idx;
    bar.start_ms = tempo_map->TickToMs(bar_start);
    bar.end_ms = tempo_map->TickToMs(bar_end);
    bars.push_back(bar);

    cur_tick = bar_end;
  }

  return bars;
}
