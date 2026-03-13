#pragma once
#include <functional>
#include "Macros.h"

class EventLoop;
class Socket;
class Channel {
 public:
  Channel(EventLoop *loop, Socket *socket);
  ~Channel();

  DISALLOW_COPY_AND_MOVE(Channel);

  /// @brief 事件处理
  void HandleEvent();

  /// @brief 开启读事件监听
  void EnableRead();

  void EnableWrite();

  Socket *GetSocket();

  /// @brief 获取希望监听的事件
  uint32_t GetListenEvents();

  /// @brief 获取实际就绪的事件
  uint32_t GetReadyEvents();

  bool GetExist();

  void SetExist(bool in = true);

  /// @brief 开启 ET 模式 (默认使用LT)
  void UseET();

  /// @brief 把epoll_wait返回的就绪事件设置到Channel中
  void SetReadyEvents(int ev);

  /// @brief 设置读事件就绪后的回调函数
  void SetReadCallback(std::function<void()> const &callback);

  void SetWriteCallback(std::function<void()> const &callback);

  static const int READ_EVENT;   // NOLINT
  static const int WRITE_EVENT;  // NOLINT
  static const int ET;           // NOLINT

 private:
  EventLoop *loop_;  // 关联的loop
  Socket *socket_;
  int listen_events_;  // 期望监听的事件：用户希望在这个文件描述符上监听哪些事件
  int ready_events_;   // 实际就绪的事件：内核返回的真正发生了的事件
  bool exist_{false};
  std::function<void()> read_callback_;
  std::function<void()> write_callback_;
};
