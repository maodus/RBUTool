#pragma once
#include <unordered_set>

#include "core/chart/NoteKey.hpp"
using namespace core;

namespace core::selection {

class SelectionManager {
 public:
  bool IsSelected(const chart::NoteKey& key) const;
  bool IsEmpty() const;
  void Add(const chart::NoteKey& key);
  void Remove(const chart::NoteKey& key);
  void Toggle(const chart::NoteKey& key);

  uint32_t Count() const;
  void Clear();

  template <class Fn>
  void ForEach(Fn&& fn) const {
    for (auto& k : selections_) fn(k);
  }

 private:
  std::unordered_set<chart::NoteKey, chart::NoteKeyHash> selections_;
};

}  // namespace core::selection