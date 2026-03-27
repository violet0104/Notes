#include "Channel.h"

#include <unistd.h>

#include "EventLoop.h"

const short Channel::READ_EVENT = 1;
const short Channel::WRITE_EVENT = 2;
const short Channel::ET = 4;

Channel::Channel(int fd, EventLoop *loop)
    : fd_(fd), loop_(loop), listen_events_(0), ready_events_(0), exist_(false) {
}

Channel::~Channel() { loop_->DeleteChannel(this); }

void Channel::HandleEvent() const {
  // 处理读事件
  if (ready_events_ & READ_EVENT) {
    read_callback_();
  }

  // 处理写事件（EPOLLOUT=可写）
  if (ready_events_ & WRITE_EVENT) {
    write_callback_();
  }
}

void Channel::EnableRead() {
  listen_events_ |= READ_EVENT;
  loop_->UpdateChannel(this);
}

void Channel::EnableWrite() {
  listen_events_ |= WRITE_EVENT;
  loop_->UpdateChannel(this);
}

void Channel::DisableAll() {
  listen_events_ = 0;
  loop_->UpdateChannel(this);
}

void Channel::EnableET() {
  listen_events_ |= ET;
  loop_->UpdateChannel(this);
}

int Channel::GetFd() const { return fd_; }
short Channel::GetListenEvents() const { return listen_events_; }
short Channel::GetReadyEvents() const { return ready_events_; }

bool Channel::IsExist() const { return exist_; }

void Channel::SetExist(bool in) { exist_ = in; }

void Channel::SetReadyEvents(int ev) {
  if (ev & READ_EVENT) {
    ready_events_ |= READ_EVENT;
  }
  if (ev & WRITE_EVENT) {
    ready_events_ |= WRITE_EVENT;
  }
  if (ev & ET) {
    ready_events_ |= ET;
  }
}
void Channel::SetReadCallback(std::function<void()> const &callback) {
  read_callback_ = callback;
}
void Channel::SetWriteCallback(std::function<void()> const &callback) {
  write_callback_ = callback;
}
