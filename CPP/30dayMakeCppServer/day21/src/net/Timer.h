#pragma once
#include "Macros.h"
#include <functional>
#include <memory>

class EventLoop;
class Channel;

class Timer {
public:
  explicit Timer(EventLoop *loop);
  ~Timer();

  DISALLOW_COPY_AND_MOVE(Timer);

  // set interval_ms to 0 for a one-shot timer
  void SetTime(int timeout_ms, int interval_ms = 0);
  void SetOnTimeoutCallback(std::function<void()> fn);
  void Cancel();

private:
  void HandleRead();

  EventLoop *loop_{nullptr};
  int fd_{-1};
  std::unique_ptr<Channel> channel_;
  std::function<void()> timeout_callback_;
};
