#include "Channel.h"
#include "EventLoop.h"
#include <unistd.h>

Channel::Channel(EventLoop *_loop, int _fd)
    : loop(_loop), fd(_fd), events(0), ready(0), inEpoll(false) {}

Channel::~Channel() {
  if (fd != -1) {
    close(fd);
    fd = -1;
  }
}

void Channel::handleEvent() {
  // 处理读事件（EPOLLIN=普通读，EPOLLPRI=紧急数据读）
  if (ready & (EPOLLIN | EPOLLPRI)) {
    readCallback();
  }

  // 处理写事件（EPOLLOUT=可写）
  if (ready & EPOLLOUT) {
    writeCallback();
  }
}

void Channel::enableReading() {
  events = EPOLLIN | EPOLLPRI;
  loop->updateChannel(this);
}

void Channel::useET() { events |= EPOLLET; }

int Channel::getFd() { return fd; }

uint32_t Channel::getEvents() { return events; }
uint32_t Channel::getReady() { return ready; }

bool Channel::getInEpoll() { return inEpoll; }

void Channel::setInEpoll(bool _in) { inEpoll = _in; }

void Channel::setReady(uint32_t _ev) { ready = _ev; }

void Channel::setReadCallback(std::function<void()> _cb) { readCallback = _cb; }
