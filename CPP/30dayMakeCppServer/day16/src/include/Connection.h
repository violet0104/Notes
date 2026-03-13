#pragma once
#include <functional>
#include "Macros.h"

class EventLoop;
class Socket;
class Channel;
class Buffer;
class Connection {
 public:
  enum State {
    Invalid = 1,
    Handshaking,
    Connected,
    Closed,
    Failed,
  };
  Connection(EventLoop *_loop, Socket *_sock);
  ~Connection();
  DISALLOW_COPY_AND_MOVE(Connection);

  void Read();
  void Write();
  void Send(std::string msg);
  
  void SetDeleteConnectionCallback(std::function<void(Socket *)> const &callback);
  void SetOnConnectCallback(std::function<void(Connection *)> const &callback);
  void SetOnMessageCallback(std::function<void(Connection *)> const &callback);
  void Business();

  State GetState();
  void Close();
  void SetSendBuffer(const char *str);
  Buffer *GetSendBuffer();
  Buffer *GetReadBuffer();
  const char *SendBuffer();
  const char *ReadBuffer();
  void GetlineSendBuffer();
  Socket *GetSocket();

  void OnConnect(std::function<void()> fn);
  void OnMessage(std::function<void()> fn);

 private:
  EventLoop *loop_;
  Socket *sock_;
  Channel *channel_{nullptr};
  State state_{State::Invalid};
  Buffer *read_buffer_{nullptr};
  Buffer *send_buffer_{nullptr};
  std::function<void(Socket *)> delete_connection_callback_;

  std::function<void(Connection *)> on_connect_callback_;
  std::function<void(Connection *)> on_message_callback_;
  
  void ReadNonBlocking();
  void WriteNonBlocking();
  void ReadBlocking();
  void WriteBlocking();
};
