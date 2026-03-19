#pragma once
#include "Macros.h"
#include <functional>
#include <memory>
#include <string>

class Connection {
public:
  enum State {
    Invalid = 1,
    Handshaking,
    Connected,
    Closed,
    Failed,
  };

  Connection(int fd, EventLoop *loop);
  ~Connection();
  DISALLOW_COPY_AND_MOVE(Connection);

  void SetDeleteConnectionCallback(std::function<void(int)> const &fn);
  void SetOnRecvCallback(std::function<void(Connection *)> const &fn);

  State GetState();
  Socket *GetSocket();

  RC Read();
  RC Write();
  RC Send(std::string msg);

  void SetSendBuffer(const char *str);
  Buffer *GetSendBuffer();
  Buffer *GetReadBuffer();

  void Close();

private:
  void Business();
  RC ReadNonBlocking();
  RC WriteNonBlocking();
  RC ReadBlocking();
  RC WriteBlocking();

private:
  std::unique_ptr<Socket> socket_;
  std::unique_ptr<Channel> channel_;

  State state_;
  std::unique_ptr<Buffer> read_buf_;
  std::unique_ptr<Buffer> send_buf_;

  std::function<void(int)> delete_connection_callback_;
  std::function<void(Connection *)> on_recv_callback_;
};
