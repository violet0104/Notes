#pragma once

#include "FixedBuffer.h"
#include "Macros.h"
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

// 异步日志类，管理双缓冲逻辑和后端写线程
class AsyncLogging {
public:
  AsyncLogging(const std::string &basename, off_t rollSize,
               int flushInterval = 3);
  ~AsyncLogging();

  // 前端调用：向当前缓冲区追加日志
  void Append(const char *logline, int len);

  // 停止后台线程
  void Stop();

private:
  // 后台线程函数：负责将缓冲区内容写入 LogFile
  void ThreadFunc();

  using Buffer = FixedBuffer<4000 * 1000>; // 4MB
  using BufferPtr = Buffer *;              // 使用原始指针，手动管理内存

  const int flushInterval_;
  std::atomic<bool> running_;
  const std::string basename_;
  const off_t rollSize_;
  std::thread thread_;
  std::mutex mutex_;
  std::condition_variable cond_;

  BufferPtr currentBuffer_;        // 前端当前缓冲区
  BufferPtr nextBuffer_;           // 前端备用缓冲区
  std::vector<BufferPtr> buffers_; // 待写入文件的缓冲区队列

  DISALLOW_COPY_AND_MOVE(AsyncLogging);
};
