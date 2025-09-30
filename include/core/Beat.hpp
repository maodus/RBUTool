#pragma once
#include <vector>
#include <core/BinaryIO.hpp>

struct BeatInfo {
	int tick;
	int level;
};

class BeatMap : IWritable {
private:
	std::vector<BeatInfo> _beats;
public:
	void AddBeat(int tick, int level);
	void write(BinaryWriter& w) const override {
		w.U32(_beats.size());
		for (const auto& beat : _beats) {
			w.U32(beat.tick);
			w.U32(beat.level);
		}
	}
};