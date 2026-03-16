#pragma once
#include "common.h"
#include <vector>

#ifdef OS_LINUX
#include <sys/epoll.h>
#endif

#ifdef OS_MACOS
#include <sys/event.h>
#endif

class Channel;
class Poller {
public:
  Poller();
  ~Poller();

  DISALLOW_COPY_AND_MOVE(Poller);

  void UpdateChannel(Channel *ch) const;
  void DeleteChannel(Channel *ch) const;

  std::vector<Channel *> Poll(int timeout = -1) const;

private:
  int fd_;

#ifdef OS_LINUX
  struct epoll_event *events_{nullptr};
#endif

#ifdef OS_MACOS
  struct kevent *events_{nullptr};
#endif
};
