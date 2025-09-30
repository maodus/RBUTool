#pragma once
#include <vector>
#include <core/BinaryIO.hpp>

struct TempoPoint {
	float ms;	// Milisecond timestamp of tempo change
	int tick;	// Starting tick
	int tempo;	// Microseconds per quarter note
};

class TempoMap : IWritable {
public:
	TempoMap(int tpq);

	void AddTempo(int tick, int tempo);
	float TickToMs(int tick) const;
	uint32_t GetTPQ() const;

	
	void write(BinaryWriter& w) const override {
		w.U32(_tempos.size());
		for (const auto& tp : _tempos) {
			w.F32(tp.ms);
			w.U32(tp.tick);
			w.U32(tp.tempo);
		}
	}
private:
	int _tpq;
	std::vector<TempoPoint> _tempos;
};