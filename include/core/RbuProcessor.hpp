#pragma once
#include <core/ChartIR.hpp>
#include <core/GameDefs.hpp>
#include <string>
#include <vector>

#include "core/chart/ChartDocument.hpp"
struct RBUHeader : IWritable {
  int version;
  char display_name[48];
  char artist_name[48];
  int difficulty;
  int genre;
  int era;
  int initial_track;
  uint32_t drum_rating;
  uint32_t guitar_rating;
  uint32_t bass_rating;
  uint32_t vocal_rating;
  uint32_t band_rating;
  char reserved[124];

  void write(BinaryWriter& w) const override {
    w.U32(version);
    w.Bytes(display_name, 48);
    w.Bytes(artist_name, 48);
    w.U32(difficulty);
    w.U32(genre);
    w.U32(era);
    w.U32(initial_track);
    w.U32((drum_rating & 0xF) | (guitar_rating & 0xF) << 4 |
          (bass_rating & 0xF) << 8 | (vocal_rating & 0xF) << 12 |
          (band_rating & 0xF) << 16);
    w.Bytes(reserved, 124);
  }
};

class RBUProcessor {
 private:
  std::vector<SoloPhrase> BuildGemTrackSolos(std::vector<GameGem>& gems) const;
  std::vector<BarInfo> BuildGemTrackBars(
      std::vector<GameGem>& gems, std::shared_ptr<TempoMap> tempo_map,
      std::shared_ptr<MeasureMap> measure_map, uint32_t last_tick) const;

 public:
  void WriteRBUFile(std::string filepath,
                    const core::chart::ChartDocument& chart,
                    std::shared_ptr<RBUHeader> header);
};
