#pragma once
#include "Macros.h"
#include <functional>

class Channel {
public:
  Channel(int fd, EventLoop *loop);
  ~Channel();

  DISALLOW_COPY_AND_MOVE(Channel);

  /// @brief 事件处理
  void HandleEvent() const;

  /// @brief 开启读事件监听
  void EnableRead();

  void EnableWrite();

  int GetFd() const;

  /// @brief 获取希望监听的事件
  short GetListenEvents() const;

  /// @brief 获取实际就绪的事件
  short GetReadyEvents() const;

  bool IsExist() const;

  void SetExist(bool in = true);

  /// @brief 开启 ET 模式 (默认使用LT)
  void EnableET();

  /// @brief 把epoll_wait返回的就绪事件设置到Channel中
  void SetReadyEvents(int ev);

  /// @brief 设置读事件就绪后的回调函数
  void SetReadCallback(std::function<void()> const &callback);

  void SetWriteCallback(std::function<void()> const &callback);

  static const short READ_EVENT;  // NOLINT
  static const short WRITE_EVENT; // NOLINT
  static const short ET;          // NOLINT

private:
  int fd_;
  EventLoop *loop_; // 关联的loop
  short
      listen_events_; // 期望监听的事件：用户希望在这个文件描述符上监听哪些事件
  short ready_events_; // 实际就绪的事件：内核返回的真正发生了的事件
  bool exist_;
  std::function<void()> read_callback_;
  std::function<void()> write_callback_;
};
