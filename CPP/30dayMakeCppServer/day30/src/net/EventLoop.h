#pragma once
#include "Macros.h"
#include <atomic>
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
  void Quit();

private:
  void HandleRead(); // 处理 wakeup_fd_ 的读取事件
  void Wakeup();     // 向 wakeup_fd_ 写入数据以唤醒

  std::unique_ptr<Poller> poller_;
  std::unique_ptr<TimerManager> timer_manager_;
  std::atomic<bool> quit_{false};
  int wakeup_fd_;                           // eventfd 用于唤醒
  std::unique_ptr<Channel> wakeup_channel_; // 处理 wakeup_fd_ 的 Channel
};
