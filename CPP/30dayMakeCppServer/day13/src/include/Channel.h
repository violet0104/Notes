#pragma once
#include <functional>
#include <sys/epoll.h>

class EventLoop;
class Channel {
private:
  EventLoop *loop;
  int fd;
  uint32_t events; // 期望监听的事件：用户希望在这个文件描述符上监听哪些事件
  uint32_t ready; // 实际就绪的事件：内核返回的真正发生了的事件
  bool inEpoll;
  std::function<void()> readCallback;
  std::function<void()> writeCallback;

public:
  Channel(EventLoop *_loop, int _fd);
  ~Channel();

  /// @brief 事件处理
  void handleEvent();

  /// @brief 开启读事件监听
  void enableReading();

  int getFd();

  /// @brief 获取关心的事件
  uint32_t getEvents();

  /// @brief 获取就绪的事件
  uint32_t getReady();

  /// @brief 判断fd是否已注册到epoll
  bool getInEpoll();

  /// @brief 标记fd已注册到epoll
  void setInEpoll(bool _in = true);

  /// @brief 开启 ET 模式 (默认使用LT)
  void useET();

  /// @brief 把epoll_wait返回的就绪事件设置到Channel中
  void setReady(uint32_t);

  /// @brief 设置读事件就绪后的回调函数
  void setReadCallback(std::function<void()>);
};
