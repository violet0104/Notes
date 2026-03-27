#include "Poller.h"

#include <cerrno>
#include <cstring>
#include <unistd.h>

#include "Channel.h"
#include "util.h"

#define MAX_EVENTS_ 1000

#ifdef OS_LINUX

Poller::Poller() {
  fd_ = epoll_create1(0);
  ErrorIf(fd_ == -1, "eproll create error");
  events_ = new epoll_event[MAX_EVENTS_];
  std::memset(events_, 0, sizeof(*events_) * MAX_EVENTS_);
}

Poller::~Poller() {
  if (fd_ != -1) {
    close(fd_);
  }
  delete[] events_;
}

std::vector<Channel *> Poller::Poll(int timeout) const {
  std::vector<Channel *> active_channels;
  int nfds = epoll_wait(fd_, events_, MAX_EVENTS_, timeout);
  if (nfds == -1) {
    if (errno == EINTR) {
      return active_channels; // 返回空列表，并在外层判断退出状态
    }
    ErrorIf(true, "epoll wait error");
  }
  for (int i = 0; i < nfds; ++i) {
    Channel *ch = (Channel *)events_[i].data.ptr;
    int events = events_[i].events;
    if (events & EPOLLIN) {
      ch->SetReadyEvents(Channel::READ_EVENT);
    }
    if (events & EPOLLOUT) {
      ch->SetReadyEvents(Channel::WRITE_EVENT);
    }
    if (events & EPOLLET) {
      ch->SetReadyEvents(Channel::ET);
    }
    active_channels.push_back(ch);
  }
  return active_channels;
}

void Poller::UpdateChannel(Channel *ch) const {
  int sockfd = ch->GetFd();
  struct epoll_event ev{};
  ev.data.ptr = ch;
  if (ch->GetListenEvents() & Channel::READ_EVENT) {
    ev.events |= EPOLLIN | EPOLLPRI;
  }
  if (ch->GetListenEvents() & Channel::WRITE_EVENT) {
    ev.events |= EPOLLOUT;
  }
  if (ch->GetListenEvents() & Channel::ET) {
    ev.events |= EPOLLET;
  }
  if (!ch->IsExist()) {
    ErrorIf(epoll_ctl(fd_, EPOLL_CTL_ADD, sockfd, &ev) == -1,
            "epoll add error");
    ch->SetExist();
  } else {
    ErrorIf(epoll_ctl(fd_, EPOLL_CTL_MOD, sockfd, &ev) == -1,
            "epoll modify error");
  }
}

void Poller::DeleteChannel(Channel *ch) const {
  int sockfd = ch->GetFd();
  if (epoll_ctl(fd_, EPOLL_CTL_DEL, sockfd, nullptr) == -1) {
    // During shutdown, the socket or epoll fd might already be closed.
    // ENOENT means it was already removed (likely because the socket was closed).
    // EBADF means the fd is already closed.
    if (errno != ENOENT && errno != EBADF) {
      ErrorIf(true, "epoll delete error");
    }
  }
  ch->SetExist(false);
}

#endif

#ifdef OS_MACOS

#include "Logger.h"

Poller::Poller() {
  fd_ = kqueue();
  assert(fd_ != -1);
  events_ = new struct kevent[MAX_EVENTS];
  memset(events_, 0, sizeof(*events_) * MAX_EVENTS);
}

Poller::~Poller() {
  if (fd_ != -1) {
    close(fd_);
    fd_ = -1;
  }
}

std::vector<Channel *> Poller::Poll(long timeout) const {
  std::vector<Channel *> active_channels;
  struct timespec ts;
  memset(&ts, 0, sizeof(ts));
  if (timeout != -1) {
    ts.tv_sec = timeout / 1000;
    ts.tv_nsec = (timeout % 1000) * 1000 * 1000;
  }
  int nfds = 0;
  if (timeout == -1) {
    nfds = kevent(fd_, NULL, 0, events_, MAX_EVENTS, NULL);
  } else {
    nfds = kevent(fd_, NULL, 0, events_, MAX_EVENTS, &ts);
  }

  for (int i = 0; i < nfds; ++i) {
    Channel *ch = (Channel *)events_[i].udata;
    int events = events_[i].filter;
    if (events == EVFILT_READ) {
      ch->set_ready_event(ch->READ_EVENT | ch->ET);
    }
    if (events == EVFILT_WRITE) {
      ch->set_ready_event(ch->WRITE_EVENT | ch->ET);
    }
    active_channels.push_back(ch);
  }
  return active_channels;
}

RC Poller::UpdateChannel(Channel *ch) const {
  struct kevent ev[2];
  memset(ev, 0, sizeof(*ev) * 2);
  int n = 0;
  int fd = ch->fd();
  int op = EV_ADD;
  if (ch->listen_events() & ch->ET) {
    op |= EV_CLEAR;
  }
  if (ch->listen_events() & ch->READ_EVENT) {
    EV_SET(&ev[n++], fd, EVFILT_READ, op, 0, 0, ch);
  }
  if (ch->listen_events() & ch->WRITE_EVENT) {
    EV_SET(&ev[n++], fd, EVFILT_WRITE, op, 0, 0, ch);
  }
  int r = kevent(fd_, ev, n, NULL, 0, NULL);
  if (r == -1) {
    LOG_ERROR("kqueue add event error");
    return RC_POLLER_ERROR;
  }
  return RC_SUCCESS;
}

RC Poller::DeleteChannel(Channel *ch) const {
  struct kevent ev[2];
  int n = 0;
  int fd = ch->fd();
  if (ch->listen_events() & ch->READ_EVENT) {
    EV_SET(&ev[n++], fd, EVFILT_READ, EV_DELETE, 0, 0, ch);
  }
  if (ch->listen_events() & ch->WRITE_EVENT) {
    EV_SET(&ev[n++], fd, EVFILT_WRITE, EV_DELETE, 0, 0, ch);
  }
  int r = kevent(fd_, ev, n, NULL, 0, NULL);
  if (r == -1) {
    LOG_ERROR("kqueue delete event error");
    return RC_POLLER_ERROR;
  }
  return RC_SUCCESS;
}
#endif
