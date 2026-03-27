#include "EventLoop.h"
#include "TimerManager.h"
#include <chrono>
#include <iostream>

int main() {
  EventLoop *loop = new EventLoop();
  TimerManager *manager = new TimerManager(loop);

  auto start = std::chrono::steady_clock::now();

  auto print_time = [start](const std::string &msg) {
    auto now = std::chrono::steady_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - start)
                  .count();
    std::cout << "[" << ms << "ms] " << msg << std::endl;
  };

  print_time("Adding timers...");

  manager->AddTimer(1000, [&]() { print_time("Timer 1 expired (1000ms)"); });

  manager->AddTimer(3000, [&]() { print_time("Timer 3 expired (3000ms)"); });

  manager->AddTimer(2000, [&]() { print_time("Timer 2 expired (2000ms)"); });

  manager->AddTimer(500, [&]() { print_time("Timer 0 expired (500ms)"); });

  std::cout << "Starting EventLoop, expect timers to fire at 500ms, 1000ms, "
               "2000ms, 3000ms"
            << std::endl;

  // 运行一段时间后手动退出测试（本框架 Loop 是死循环，实际测试中需要 ctrl+c
  // 或在回调中 exit） 这里我们在最后一个定时器执行时 exit
  manager->AddTimer(3500, [&]() {
    print_time("All tests finished, exiting...");
    exit(0);
  });

  loop->Loop();

  return 0;
}
