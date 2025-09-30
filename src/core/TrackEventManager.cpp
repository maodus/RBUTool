#include <algorithm>
#include <core/TrackEventManager.hpp>
#include <iostream>

// return pair of iterators covering the desired range
static std::pair<std::vector<GameGem>::iterator, std::vector<GameGem>::iterator>
FindGemRange(std::vector<GameGem>& gems, int tick_on, int tick_off) {
  // assumes notes sorted by 'on'
  auto first = std::lower_bound(
      gems.begin(), gems.end(), tick_on,
      [](const GameGem& gem, int value) { return gem.tick < value; });

  // we want last note with off <= desiredEnd
  auto last = std::upper_bound(
      gems.begin(), gems.end(), tick_off,
      [](int value, const GameGem& gem) { return value < gem.tick; });

  return {first, last};  // [first, last)
}

TrackEventManager::TrackEventManager(const EventList& events)
    : track_events_(events) {}

std::vector<TrackEvent> TrackEventManager::GetEventsForTick(int tick,
                                                            int track) const {
  std::vector<TrackEvent> active_events;

  for (const auto& event : track_events_.events) {
    if (tick >= event.on && tick <= event.off) {
      active_events.push_back(event);
    }
  }

  return active_events;
}

void TrackEventManager::ApplyEvents(std::vector<GameGem>& gems,
                                    int track) const {
  for (const auto& event : track_events_.events) {
    auto [first_gem, last_gem] = FindGemRange(gems, event.on, event.off);
    if (first_gem == gems.end() || last_gem == gems.end()) {
      continue;
    }

    std::cout << last_gem->tick << " " << event.on << std::endl;
    const std::size_t start_idx = std::distance(gems.begin(), first_gem);
    const std::size_t end_idx = std::distance(gems.begin(), last_gem);

    if (event.type == EventKind::Solo) {
      const int gem_count = end_idx - start_idx + 1;
      first_gem->MakeSoloStart();
      first_gem->SetCaptures(gem_count);
      for (auto gem_it = std::next(first_gem); gem_it < last_gem; ++gem_it) {
        gem_it->MakeSolo();
      }
      last_gem->MakeSoloEnd();
    } else if (event.type == EventKind::Overdrive) {
      for (auto gem_it = first_gem; gem_it < last_gem; ++gem_it) {
        gem_it->MakeOverDrive();
      }
      last_gem->MakeOverDriveEnd();
    }
  }
}
