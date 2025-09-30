#pragma once
#include <core/post/PostPass.hpp>

class GemPruner : public IPostPass {
 public:
  std::vector<core::chart::ViewNote> Run(
      const PostContext& ctx,
      std::vector<core::chart::ViewNote>& notes) const override;
};

class SparsifyMultiGemPass : public IPostPass {
 public:
  std::vector<core::chart::ViewNote> Run(
      const PostContext& ctx,
      std::vector<core::chart::ViewNote>& notes) const override;
};

class SeperateMultiGemPass : public IPostPass {
 public:
  std::vector<core::chart::ViewNote> Run(
      const PostContext& ctx,
      std::vector<core::chart::ViewNote>& notes) const override;
};