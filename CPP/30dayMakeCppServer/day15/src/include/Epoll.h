#pragma once
#ifdef OS_LINUX
#include <sys/epoll.h>
#endif
#include <vector>

#include "Channel.h"
#include "Macros.h"

class Channel;
class Epoll {
 public:
  Epoll();
  ~Epoll();

  DISALLOW_COPY_AND_MOVE(Epoll);

  void UpdateChannel(Channel *);
  void DeleteChannel(Channel *);

  std::vector<Channel *> Poll(int timeout = -1);

 private:
  int epfd_{1};
  struct epoll_event *events_{nullptr};
};
