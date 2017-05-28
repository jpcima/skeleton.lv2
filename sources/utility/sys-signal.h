#pragma once

class signal_blocker {
 public:
  explicit signal_blocker(int sig) noexcept;
  ~signal_blocker();
  void activate();
  void deactivate() noexcept;
 private:
  const int sig_ {};
  bool masked_ {}, active_ {};
 private:
  signal_blocker(const signal_blocker &&) = delete;
  signal_blocker &operator=(const signal_blocker &&) = delete;
};

#include "sys-signal.tcc"
