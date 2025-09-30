#include <core/Tempo.hpp>

TempoMap::TempoMap(int tpq) : _tpq(tpq)
{
	_tempos.push_back({ 0, 0, 500000 });
}

static float MsPerTick(int tempo, int tpq) {
	return (static_cast<float>(tempo) / 1000.0) / static_cast<float>(tpq);
}

void TempoMap::AddTempo(int tick, int tempo) {
	auto num_tempos = _tempos.size();
	if (num_tempos == 1 && tick == 0) {
		_tempos[0] = {0, 0, tempo};
	} else {
		const auto& last_tempo = _tempos.back();
		const int t_delta = tick - last_tempo.tick; // Delta time in ticks
		const float new_ms = last_tempo.ms + t_delta * MsPerTick(last_tempo.tempo, _tpq);
		_tempos.push_back({ new_ms, tick, tempo });
	}
}

float TempoMap::TickToMs(int tick) const {
	if (_tempos.empty() || tick < _tempos.front().tick) {
		return 0.0;
	}

	int lo = 0, hi = _tempos.size();

	// Binary search to find tempo segment O(logn)
	while (lo + 1 < hi) {
		int mid = (lo + hi) / 2;
		if (_tempos[mid].tick <= tick) {
			lo = mid;
		} else {
			hi = mid;
		}
	}

	const auto& point = _tempos[lo];
	const int t_delta = tick - point.tick;
	return point.ms + t_delta * MsPerTick(point.tempo, _tpq);
}

uint32_t TempoMap::GetTPQ() const {
	return this->_tpq;
}
