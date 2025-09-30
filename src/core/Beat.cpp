#include <core/Beat.hpp>

void BeatMap::AddBeat(int tick, int level) {
	_beats.push_back({ tick, (level == 12) ? 1 : 0 });
}
