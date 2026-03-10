#ifndef CPP_30DAYMAKECPPSERVER_DAY13_SRC_INCLUDE_EPOLL_H_
#define CPP_30DAYMAKECPPSERVER_DAY13_SRC_INCLUDE_EPOLL_H_

#include <sys/epoll.h>
#include <vector>

#include "Channel.h"

class Channel;
class Epoll {
private:
  int epfd;
  struct epoll_event *events;

public:
  Epoll();
  ~Epoll();

  void updateChannel(Channel *);
  void deleteChannel(Channel *);

  std::vector<Channel *> poll(int timeout = -1);
};

#endif // CPP_30DAYMAKECPPSERVER_DAY13_SRC_INCLUDE_EPOLL_H_
