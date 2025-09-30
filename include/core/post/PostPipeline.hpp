#pragma once
#include <core/post/PostPass.hpp>
#include <vector>

#include "core/post/Passes.hpp"

class PostPipeline {
 public:
  PostPipeline();
  std::vector<core::chart::ViewNote> BuildView(
      const core::chart::ChartDocument& chart_doc, int diff, int track,
      const PostConfig& cfg) const;

 private:
  std::vector<std::unique_ptr<IPostPass>> passes_;
};