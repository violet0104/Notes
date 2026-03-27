#include "EventLoop.h"

#include <vector>

#include "Channel.h"
#include "Poller.h"
#include "TimerManager.h"

EventLoop::EventLoop() {
  poller_ = std::unique_ptr<Poller>(new Poller());
  timer_manager_ = std::unique_ptr<TimerManager>(new TimerManager(this));
}

EventLoop::~EventLoop() {}

void EventLoop::Loop() {
  while (!quit_) {
    std::vector<Channel *> active_channels;
    active_channels = poller_->Poll(100);
    if (quit_)
      break;
    for (auto &active_channel : active_channels) {
      active_channel->HandleEvent();
    }
  }
}

void EventLoop::UpdateChannel(Channel *ch) { poller_->UpdateChannel(ch); }
void EventLoop::DeleteChannel(Channel *ch) { poller_->DeleteChannel(ch); }

void EventLoop::AddTimer(int timeout_ms, std::function<void()> fn) {
  timer_manager_->AddTimer(timeout_ms, std::move(fn));
}

void EventLoop::Quit() {
  quit_ = true;
  // TODO: 如果在 epoll_wait 阻塞中，可能需要 notify 来唤醒。
  // 但目前我们的 Poller::Poll() 应该有超时机制或者可以通过发送信号/写入 eventfd 唤醒。
}
