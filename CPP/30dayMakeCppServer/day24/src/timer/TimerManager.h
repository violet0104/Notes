#pragma once
#include "Macros.h"
#include <chrono> // NOLINT
#include <functional>
#include <memory>
#include <queue>
#include <vector>

class EventLoop;
class Channel;

// Timer 内部节点信息
struct TimerNode {
  std::chrono::steady_clock::time_point expire_time; // 到期时间
  std::function<void()> callback;                    // 回调函数

  // 优先级队列使用小根堆，需要定义 > 运算符
  bool operator>(const TimerNode &other) const {
    return expire_time > other.expire_time;
  }
};

class TimerManager {
public:
  explicit TimerManager(EventLoop *loop);
  ~TimerManager();

  DISALLOW_COPY_AND_MOVE(TimerManager);

  /**
   * @brief 添加一个定时任务
   * @param timeout_ms 多少毫秒后触发
   * @param fn 触发时的回调函数
   */
  void AddTimer(int timeout_ms, std::function<void()> fn);

private:
  void HandleRead();
  void ResetTimerFd();

  EventLoop *loop_{nullptr};
  int timer_fd_{-1};
  std::unique_ptr<Channel> timer_channel_;

  // 小根堆管理所有定时器任务
  std::priority_queue<TimerNode, std::vector<TimerNode>,
                      std::greater<TimerNode>>
      timers_;
};
