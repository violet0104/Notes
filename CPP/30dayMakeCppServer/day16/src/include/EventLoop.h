#pragma once
#include "common.h"
#include <memory>

class Poller;
class Channel;
class EventLoop {
public:
  EventLoop();
  ~EventLoop();

  DISALLOW_COPY_AND_MOVE(EventLoop);

  void Loop();
  void UpdateChannel(Channel *ch);
  void DeleteChannel(Channel *ch);

private:
  std::unique_ptr<Poller> poller_;
};
