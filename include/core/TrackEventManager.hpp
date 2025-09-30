#pragma once

#include <core/ChartIR.hpp>
#include <core/GameDefs.hpp>
#include <vector>

class TrackEventManager {
public:
	TrackEventManager(const EventList& events);
	std::vector<TrackEvent> GetEventsForTick(int tick, int track) const;
	void ApplyEvents(std::vector<GameGem>& gems, int track) const;
	

private:
	const EventList& track_events_;
};