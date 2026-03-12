#include "ThreadPool.h"

ThreadPool::ThreadPool(unsigned int size) {
  for (unsigned int i = 0; i < size; ++i) {
    workers_.emplace_back(std::thread([this]() {
      // 线程工作逻辑
      while (true) {
        std::function<void()> task; // 存储取出的单个任务
        {
          std::unique_lock<std::mutex> lock(queue_mutex_);

          // 等待条件变量：阻塞当前线程，直到 线程池停止 或 任务队列不为空
          condition_variable_.wait(lock, [this]() { return stop_ || !tasks_.empty(); });

          // 如果 线程池停止 且 任务队列为空，退出线程
          if (stop_ && tasks_.empty())
            return;

          // 从任务队列取出一个任务
          task = tasks_.front();
          tasks_.pop();
        }
        task();
      }
    }));
  }
}

ThreadPool::~ThreadPool() {
  {
    std::unique_lock<std::mutex> lock(queue_mutex_);
    stop_ = true;
  }

  // 唤醒所有阻塞的工作线程，让它们判断退出条件
  condition_variable_.notify_all();

  // 等待所有工作线程执行完毕，回收线程资源
  for (std::thread &th : workers_) {
    if (th.joinable()) {
      th.join();
    }
  }
}
