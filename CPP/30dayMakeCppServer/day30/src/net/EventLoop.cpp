#include "EventLoop.h"
#include "Channel.h"
#include "Poller.h"
#include "TimerManager.h"
#include <sys/eventfd.h>
#include <unistd.h>
#include <vector>

EventLoop::EventLoop() {
  poller_ = std::unique_ptr<Poller>(new Poller());
  timer_manager_ = std::unique_ptr<TimerManager>(new TimerManager(this));

  wakeup_fd_ = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
  wakeup_channel_ = std::unique_ptr<Channel>(new Channel(wakeup_fd_, this));
  wakeup_channel_->SetReadCallback(std::bind(&EventLoop::HandleRead, this));
  wakeup_channel_->EnableRead();
}

EventLoop::~EventLoop() {
  if (wakeup_fd_ != -1) {
    close(wakeup_fd_);
  }
}

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
  Wakeup(); // 立即唤醒 epoll_wait
}

void EventLoop::HandleRead() {
  uint64_t one = 1;
  ssize_t n = read(wakeup_fd_, &one, sizeof(one));
  (void)n;
}

void EventLoop::Wakeup() {
  uint64_t one = 1;
  ssize_t n = write(wakeup_fd_, &one, sizeof(one));
  (void)n;
}
