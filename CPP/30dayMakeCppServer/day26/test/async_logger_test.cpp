#include "Logger.h"
#include <chrono>
#include <iostream>
#include <thread>
#include <vector>

void bench() {
  int cnt = 1000000;
  const char *msg = "This is a log message for testing performance.";

  auto start = std::chrono::high_resolution_clock::now();
  for (int i = 0; i < cnt; ++i) {
    LOG_INFO("%s %06d", msg, i);
  }
  auto end = std::chrono::high_resolution_clock::now();
  auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
  std::cout << "Async logger benchmark: " << cnt
            << " logs took " << diff.count() << " ms (" 
            << (cnt * 1000.0 / diff.count()) << " logs/sec)\n";
}

int main() {
  Logger::Instance().SetLevel(LogLevel::INFO);
  std::cout << "Starting async logger benchmark...\n";
  
  // 单线程压测
  bench();

  // 多线程写日志测试
  std::cout << "Starting multi-threaded log test (4 threads, 100k per thread)...\n";
  std::vector<std::thread> workers;
  for (int i = 0; i < 4; ++i) {
    workers.emplace_back([i]() {
      for (int j = 0; j < 100000; ++j) {
        LOG_INFO("Worker thread %d log message #%d", i, j);
      }
    });
  }

  for (auto &t : workers) {
    t.join();
  }
  
  std::cout << "All logs written. Check pine_server.log for results.\n";
  return 0;
}
