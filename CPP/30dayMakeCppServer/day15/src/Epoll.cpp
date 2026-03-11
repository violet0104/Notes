#include <cstring>
#include <unistd.h>

#include "Channel.h"
#include "Epoll.h"
#include "util.h"

#define MAX_events_ 1000

Epoll::Epoll() {
  epfd_ = epoll_create1(0);
  ErrorIf(epfd_ == -1, "eproll create error");
  events_ = new epoll_event[MAX_events_];
  std::memset(events_, 0, sizeof(*events_) * MAX_events_);
}

Epoll::~Epoll() {
  if (epfd_ != -1) {
    close(epfd_);
    epfd_ = -1;
  }
  delete[] events_;
}

std::vector<Channel *> Epoll::Poll(int timeout) {
  std::vector<Channel *> activeChannels;
  int nfds = epoll_wait(epfd_, events_, MAX_events_, timeout);
  ErrorIf(nfds == -1, "epoll wait error");
  for (int i = 0; i < nfds; ++i) {
    Channel *ch = (Channel *)events_[i].data.ptr;
    ch->SetReadyEvents(events_[i].events);
    activeChannels.push_back(ch);
  }
  return activeChannels;
}

void Epoll::UpdateChannel(Channel *channel) {
  int fd = channel->GetFd();
  struct epoll_event ev{};
  ev.data.ptr = channel;
  ev.events = channel->GetListenEvents();
  if (!channel->GetInEpoll()) {
    ErrorIf(epoll_ctl(epfd_, EPOLL_CTL_ADD, fd, &ev) == -1, "epoll add error");
    channel->SetInEpoll();
  } else {
    ErrorIf(epoll_ctl(epfd_, EPOLL_CTL_MOD, fd, &ev) == -1, "epoll modify error");
  }
}

void Epoll::DeleteChannel(Channel *channel) {
  int fd = channel->GetFd();
  ErrorIf(epoll_ctl(epfd_, EPOLL_CTL_DEL, fd, NULL) == -1, "epoll delete error");
  channel->SetInEpoll(false);
}
