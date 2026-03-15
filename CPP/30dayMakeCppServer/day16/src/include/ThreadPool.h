#pragma once
#include <condition_variable>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>

class ThreadPool {
public:
  /// @brief 创建工作线程
  explicit ThreadPool(unsigned int size = std::thread::hardware_concurrency());
  ~ThreadPool();

  DISALLOW_COPY_AND_MOVE(ThreadPool);

  // void add(std::function<void()>);
  template <class F, class... Args>
  auto Add(F &&f, Args &&...args) -> std::future<
      std::invoke_result_t<F, Args...>>; // 返回值后置：推导任务返回值并封装为

private:
  std::vector<std::thread> workers_;        // 工作线程
  std::queue<std::function<void()>> tasks_; // 任务队列
  std::mutex queue_mutex_; // 互斥锁：用于线程互斥访问任务的队列
  std::condition_variable
      condition_variable_;        // 条件变量：实现工作线程的阻塞和唤醒
  std::atomic<bool> stop_{false}; // 线程停止标志 // future
};

// 不能放在cpp文件，原因是C++编译器不支持模版的分离编译
template <class F, class... Args>
auto ThreadPool::Add(F &&f, Args &&...args)
    -> std::future<std::invoke_result_t<F, Args...>> {
  using return_type = std::invoke_result_t<F, Args...>;

  auto task = std::make_shared<std::packaged_task<return_type()>>(
      std::bind(std::forward<F>(f), std::forward<Args>(args)...));

  std::future<return_type> res = task->get_future();
  {
    std::unique_lock<std::mutex> lock(queue_mutex_);

    // 线程池停止后，不允许再进入任务队列
    if (stop_) {
      throw std::runtime_error("enqueue on stopped ThreadPool");
    }
    // 把有返回值的packaged_task，转换成任务队列能接收的std::function<void()>类型。
    // [task]是 lambda 的捕获列表，作用是让 lambda 能访问外部的变量task；
    // 按值捕获能保证 lambda 有自己的task副本，避免外部task被销毁后 lambda
    // 访问野指针
    tasks_.emplace([task]() { (*task)(); });
  }
  condition_variable_.notify_one();
  return res;
}
