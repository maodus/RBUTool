#pragma once
#include <gui\ChartMetaView.hpp>
#include <gui\ChartWindow.hpp>

#include "gui/AudioConversion.hpp"

class App {
 private:
  std::unique_ptr<ChartMetaView> chart_meta_window_;
  std::unique_ptr<ChartWindow> chart_window_;
  std::unique_ptr<AudioConversion> audio_window_;

  void DrawRootDockHost();
  void DrawAudioWindow();
  void DrawChartWindow();

 public:
  App();
  void render();
};