#pragma once
#include <sys/epoll.h>
#include <functional>

#include "Macros.h"

class EventLoop;
class Channel {
 public:
  Channel(EventLoop *loop, int fd);
  ~Channel();

  DISALLOW_COPY_AND_MOVE(Channel);

  /// @brief 事件处理
  void HandleEvent();

  /// @brief 开启读事件监听
  void EnableRead();

  int GetFd();

  /// @brief 获取希望监听的事件
  uint32_t GetListenEvents();

  /// @brief 获取实际就绪的事件
  uint32_t GetReadyEvents();

  /// @brief 判断fd是否已注册到epoll
  bool GetInEpoll();

  /// @brief 标记fd已注册到epoll
  void SetInEpoll(bool in = true);

  /// @brief 开启 ET 模式 (默认使用LT)
  void UseET();

  /// @brief 把epoll_wait返回的就绪事件设置到Channel中
  void SetReadyEvents(uint32_t ev);

  /// @brief 设置读事件就绪后的回调函数
  void SetReadCallback(std::function<void()> const &callback);

 private:
  EventLoop *loop_;  // 关联的loop
  int fd_;
  uint32_t listen_events_;  // 期望监听的事件：用户希望在这个文件描述符上监听哪些事件
  uint32_t ready_events_;   // 实际就绪的事件：内核返回的真正发生了的事件
  bool in_epoll_;
  std::function<void()> read_callback_;
  std::function<void()> write_callback_;
};
