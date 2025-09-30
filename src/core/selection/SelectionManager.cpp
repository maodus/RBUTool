#include "core/selection/SelectionManager.hpp"

namespace core::selection {
bool SelectionManager::IsSelected(const chart::NoteKey& key) const {
  return selections_.find(key) != selections_.end();
}

bool SelectionManager::IsEmpty() const { return selections_.empty(); }

void SelectionManager::Add(const chart::NoteKey& key) {
  selections_.insert(key);
}

void SelectionManager::Remove(const chart::NoteKey& key) {
  selections_.erase(key);
}

void SelectionManager::Toggle(const chart::NoteKey& key) {
  if (IsSelected(key)) {
    selections_.erase(key);
  } else {
    Add(key);
  }
}

uint32_t SelectionManager::Count() const { return selections_.size(); }

void SelectionManager::Clear() { selections_.clear(); }
}  // namespace core::selection