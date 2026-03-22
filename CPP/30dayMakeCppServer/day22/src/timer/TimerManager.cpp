#include "TimerManager.h"
#include "Channel.h"
#include "EventLoop.h"
#include <cassert>
#include <cstdio>
#include <string.h>
#include <sys/timerfd.h>
#include <unistd.h>

TimerManager::TimerManager(EventLoop *loop) : loop_(loop) {
  timer_fd_ = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
  assert(timer_fd_ != -1);
  timer_channel_ = std::unique_ptr<Channel>(new Channel(timer_fd_, loop_));

  timer_channel_->SetReadCallback(std::bind(&TimerManager::HandleRead, this));
  timer_channel_->EnableRead();
}

TimerManager::~TimerManager() {
  if (timer_fd_ != -1) {
    close(timer_fd_);
  }
}

void TimerManager::AddTimer(int timeout_ms, std::function<void()> fn) {
  // 获取当前时间
  auto now = std::chrono::steady_clock::now();
  // 计算过期时间
  auto expire = now + std::chrono::milliseconds(timeout_ms);

  bool is_earliest = false;
  if (timers_.empty() || expire < timers_.top().expire_time) {
    is_earliest = true;
  }

  timers_.push({expire, std::move(fn)});

  // 如果新加入的是最早到期的，则需要立即更新内核定时器到期时间
  if (is_earliest) {
    ResetTimerFd();
  }
}

void TimerManager::HandleRead() {
  uint64_t exp;
  ssize_t s = read(timer_fd_, &exp, sizeof(uint64_t));
  if (s != sizeof(uint64_t)) {
    // printf("TimerManager read error\n");
  }

  auto now = std::chrono::steady_clock::now();

  // 处理所有已超时的任务
  while (!timers_.empty() && timers_.top().expire_time <= now) {
    TimerNode node = timers_.top();
    timers_.pop();
    if (node.callback) {
      node.callback();
    }
    // 重新获取当前时间，防止回调执行太久导致后续节点在处理过程中也过期了
    now = std::chrono::steady_clock::now();
  }

  // 处理完当前过期任务后，如果队列里还有任务，则根据下一个过期时间重设内核定时器
  if (!timers_.empty()) {
    ResetTimerFd();
  }
}

// 重置内核定时器
void TimerManager::ResetTimerFd() {
  if (timers_.empty()) {
    return;
  }

  auto now = std::chrono::steady_clock::now();
  auto expire = timers_.top().expire_time;
  // 计算剩余时间：距离下个任务触发还有多久
  auto duration =
      std::chrono::duration_cast<std::chrono::milliseconds>(expire - now);

  // 防止 duration 为负数（已经过期但还没来得及触发 HandleRead）
  // 如果已经过期，就设个1毫秒，让内核赶紧叫醒event_loop处理
  int64_t ms = duration.count() > 0 ? duration.count() : 1;

  struct itimerspec new_value;
  memset(&new_value, 0, sizeof(new_value));

  new_value.it_value.tv_sec = ms / 1000;
  new_value.it_value.tv_nsec = (ms % 1000) * 1000000;

  int ret = timerfd_settime(timer_fd_, 0, &new_value, nullptr);
  assert(ret != -1);
}
