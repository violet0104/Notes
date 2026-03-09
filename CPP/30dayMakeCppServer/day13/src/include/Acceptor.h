#pragma once
#include <functional>

class EventLoop;
class Socket;
class Channel;
class Acceptor {
 private:
  EventLoop *loop;
  Socket *sock;
  Channel *acceptChannel;
  std::function<void(Socket *)> newConnectionCallback;

 public:
  explicit Acceptor(EventLoop *_loop);
  ~Acceptor();

  /// @brief 新客户端连接
  void acceptConnection();
  void setNewConnectionCallback(std::function<void(Socket *)>);
};
