#include "sys-signal.h"
#include <system_error>
#include <signal.h>
#include <errno.h>

signal_blocker::signal_blocker(int sig) noexcept
    : sig_(sig) {
}

signal_blocker::~signal_blocker() {
  deactivate();
}

void signal_blocker::activate() {
  if (active_)
    return;
  sigset_t mask, omask;
  sigemptyset(&mask);
  sigaddset(&mask, sig_);
  int ret = pthread_sigmask(SIG_BLOCK, &mask, &omask);
  if (ret != 0)
    throw std::system_error(ret, std::system_category());
  masked_ = sigismember(&omask, sig_);
  active_ = true;
}

void signal_blocker::deactivate() noexcept {
  if (!active_)
    return;
  sigset_t mask, pending;
  sigemptyset(&mask);
  sigaddset(&mask, sig_);
  sigpending(&pending);
  if (sigismember(&pending, sig_)) {
    struct timespec timeout {};
    sigtimedwait(&mask, nullptr, &timeout);
  }
  if (!masked_)
    pthread_sigmask(SIG_UNBLOCK, &mask, nullptr);
  active_ = false;
}
