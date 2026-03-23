#include "EventLoop.h"
#include "Timer.h"
#include <iostream>

int main() {
  EventLoop *loop = new EventLoop();

  Timer *timer = new Timer(loop);
  timer->SetTime(1000, 1000); // 1s timeout, 1s interval

  int count = 0;
  timer->SetOnTimeoutCallback([&count, timer]() {
    std::cout << "Timer expired! count: " << ++count << std::endl;
    if (count >= 3) {
      std::cout << "Canceling timer." << std::endl;
      timer->Cancel();
    }
  });

  std::cout << "Starting EventLoop, wait for 3 seconds..." << std::endl;
  // NOTE: since there are no connections, the event loop will just sleep until
  // timer triggers
  loop->Loop();

  delete timer;
  delete loop;
  return 0;
}
