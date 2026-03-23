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
  while (true) {
    std::vector<Channel *> active_channels;
    active_channels = poller_->Poll();
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
