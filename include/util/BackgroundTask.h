#pragma once
#include <atomic>
#include <functional>
#include <mutex>
#include <string>
#include <thread>
#include <utility>

struct ProgressReporter {
  // Report progress in [0.0f .. 1.0f]
  std::function<void(float)> set_progress;
  // Append a log line (thread-safe)
  std::function<void(const std::string&)> append_log;
  // Check for cooperative cancel
  std::function<bool()> is_cancelled;
};

class BackgroundTask {
 public:
  BackgroundTask() = default;
  ~BackgroundTask() {
    Cancel();
    Join();
  }

  BackgroundTask(const BackgroundTask&) = delete;
  BackgroundTask& operator=(const BackgroundTask&) = delete;

  template <typename Fn>
  void Start(Fn&& job) {
    Cancel();
    Join();

    cancelled_.store(false);
    completed_.store(false);
    progress_.store(0.0f);
    {
      std::lock_guard<std::mutex> lk(log_mtx_);
      log_.clear();
    }

    worker_ = std::thread([this, job_fwd = std::forward<Fn>(job)]() mutable {
      ProgressReporter rep{[this](float p) { progress_.store(p); },
                           [this](const std::string& s) {
                             std::lock_guard<std::mutex> lk(log_mtx_);
                             log_ += s;
                           },
                           [this]() { return cancelled_.load(); }};

      try {
        job_fwd(rep);
      } catch (const std::exception& e) {
        std::lock_guard<std::mutex> lk(log_mtx_);
        log_ += std::string("ERROR: ") + e.what() + "\n";
      } catch (...) {
        std::lock_guard<std::mutex> lk(log_mtx_);
        log_ += "ERROR: unknown exception\n";
      }
      completed_.store(true);
    });
  }

  void Cancel() { cancelled_.store(true); }

  void Join() {
    if (worker_.joinable()) worker_.join();
  }

  // Started and not completed yet.
  bool Running() const { return worker_.joinable() && !completed_.load(); }

  bool Completed() const { return completed_.load(); }

  float Progress() const { return progress_.load(); }

  std::string LogSnapshot() const {
    std::lock_guard<std::mutex> lk(log_mtx_);
    return log_;
  }

 private:
  std::thread worker_;
  std::atomic<bool> cancelled_{false};
  std::atomic<bool> completed_{false};
  std::atomic<float> progress_{0.0f};
  mutable std::mutex log_mtx_;
  std::string log_;
};
