#pragma once
#include "Macros.h"
#include <functional>
#include <memory>

class EventLoop {
public:
  EventLoop();
  ~EventLoop();

  DISALLOW_COPY_AND_MOVE(EventLoop);

  void Loop();
  void UpdateChannel(Channel *ch);
  void DeleteChannel(Channel *ch);

  void AddTimer(int timeout_ms, std::function<void()> fn);

private:
  std::unique_ptr<Poller> poller_;
  std::unique_ptr<TimerManager> timer_manager_;
};
