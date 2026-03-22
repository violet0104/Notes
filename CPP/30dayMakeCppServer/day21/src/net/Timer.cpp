#include "Timer.h"
#include "Channel.h"
#include "EventLoop.h"
#include <cassert>
#include <cstdio>
#include <string.h>
#include <sys/timerfd.h>
#include <unistd.h>
#include <utility>

Timer::Timer(EventLoop *loop) : loop_(loop) {
  fd_ = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
  assert(fd_ != -1);
  channel_ = std::make_unique<Channel>(fd_, loop_);

  std::function<void()> cb = std::bind(&Timer::HandleRead, this);
  channel_->SetReadCallback(cb);
  channel_->EnableRead();
}

Timer::~Timer() {
  if (fd_ != -1) {
    close(fd_);
  }
}

/// @brief 设置定时器
/// @param timeout_ms 超时时间
/// @param interval_ms 间隔时间
void Timer::SetTime(int timeout_ms, int interval_ms) {
  struct itimerspec new_value;
  memset(&new_value, 0, sizeof(new_value));

  new_value.it_value.tv_sec = timeout_ms / 1000;
  new_value.it_value.tv_nsec = (timeout_ms % 1000) * 1000000;

  if (interval_ms > 0) {
    new_value.it_interval.tv_sec = interval_ms / 1000;
    new_value.it_interval.tv_nsec = (interval_ms % 1000) * 1000000;
  }

  int ret = timerfd_settime(fd_, 0, &new_value, nullptr);
  assert(ret != -1);
}

void Timer::SetOnTimeoutCallback(std::function<void()> fn) {
  timeout_callback_ = std::move(fn);
}

void Timer::Cancel() {
  SetTime(0, 0); // setting it_value to 0 disarms the timer
}

void Timer::HandleRead() {
  uint64_t exp;
  ssize_t s = read(fd_, &exp, sizeof(uint64_t));
  if (s != sizeof(uint64_t)) {
    // maybe log error
    printf("Timer read error\n");
  }
  if (timeout_callback_) {
    timeout_callback_();
  }
}
