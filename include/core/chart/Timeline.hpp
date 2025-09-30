#pragma once

struct BeatTimeline {
  int tpq = 480;             // ticks per quarter
  double px_per_qn = 120.0;  // pixels per quarter note at zoom=1
  double zoom = 1.0;
  double scroll_qn = 0.0;  // left edge in QUARTERS (beats)

  double PxPerQn() const { return px_per_qn * zoom; }

  float TickToX(int tick, float origin_x) const {
    double qn = tick / double(tpq);
    return float(origin_x + (qn)*PxPerQn());
  }
  int XToTick(float x, float origin_x) const {
    double qn = (x - origin_x) / PxPerQn();
    return int(std::llround(qn * tpq));
  }
  void ZoomAbout(double anchor_qn, double factor) {
    factor = std::clamp(factor, 0.1, 16.0);
    double old = PxPerQn();
    zoom *= factor;
    double neu = PxPerQn();
    // keep anchor beat under mouse stationary
    scroll_qn += (anchor_qn * (neu - old)) / (old * neu);
    if (scroll_qn < 0.0) scroll_qn = 0.0;
  }
};
