#pragma once

#include <core/Common.h>

#include <core/BinaryIO.hpp>

struct GameGem : IWritable {
  float ms;
  int tick;
  short duration_ms;
  short duration_tick;
  short unknown;

  union {
    struct {
      char lane : 4;  // 1 = red, 2 = yellow, 4 = green, 8 = red
      char unknown2 : 4;
    };
    char raw_mods;
  };

  char unknown3;

  union {
    struct {
      int gem_captures : 16;  // How many gems are captured in the (solo?)
      int flags : 16;
    };

    int raw_flags;
  };

  void ToggleLane(NoteColour new_lane) {
    lane |= 1 << static_cast<char>(new_lane);
  }
  void SetCaptures(int capture_count) {
    gem_captures = (capture_count & 0xFFFF);
  }
  void MakeSolo() { flags |= 0x0800; }
  void MakeSoloStart() {
    MakeSolo();
    flags |= 0x1000;
  }
  void MakeSoloEnd() {
    MakeSolo();
    flags |= 0x2000;
  }
  void MakeOverDrive() { flags |= 0x200; }
  void MakeOverDriveEnd() {
    MakeOverDrive();
    flags |= 0x400;
  }

  bool IsSoloStart() const { return static_cast<bool>(flags & 0x1000); }
  uint32_t GetCaptures() const { return static_cast<uint32_t>(gem_captures); }

  void write(BinaryWriter& w) const override {
    w.F32(ms);
    w.U32(tick);
    w.U16(duration_ms);
    w.U16(duration_tick);
    w.U16(unknown);
    w.U8(raw_mods);
    w.U8(unknown3);
    w.U32(raw_flags);
  }
};

struct SoloPhrase : IWritable {
  float ms;
  float duration_ms;
  uint32_t tick;
  uint32_t duration_tick;
  uint32_t gems_captured;

  void write(BinaryWriter& w) const override {
    w.F32(ms);
    w.F32(duration_ms);
    w.U32(tick);
    w.U32(duration_tick);
    w.U32(gems_captured);
  }
};

struct BarInfo : IWritable {
  uint32_t start_gem_id;
  uint32_t end_gem_id;
  float start_ms;
  float end_ms;

  void write(BinaryWriter& w) const override {
    w.U32(start_gem_id);
    w.U32(end_gem_id);
    w.F32(start_ms);
    w.F32(end_ms);
  }
};