#pragma once
#include "Macros.h"
#include <functional>
#include <memory>

class Acceptor {
public:
  DISALLOW_COPY_AND_MOVE(Acceptor)
  explicit Acceptor(EventLoop *loop);
  ~Acceptor();

  /// @brief 新客户端连接
  RC AcceptConnection() const;

  void SetNewConnectionCallback(std::function<void(int)> const &callback);

private:
  std::unique_ptr<Socket> socket_;
  std::unique_ptr<Channel> channel_;
  std::function<void(int)> new_connection_callback_;
};
