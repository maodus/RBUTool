#pragma once
#include <string>

#include "core/ChartIR.hpp"
namespace core ::chart {
class IChartImporter {
 public:
  explicit IChartImporter(bool is_pitched) : is_pitched(is_pitched) {};
  virtual bool ImportChart(const std::string& file_path) = 0;
  virtual void ToGuiTrack(ChartIR& chart) = 0;

 protected:
  bool is_pitched;  // Whether to interpret the vocal track as pitched
};
}  // namespace core::chart