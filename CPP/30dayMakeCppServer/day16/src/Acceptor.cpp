#include "Acceptor.h"

#include "Channel.h"
#include "Socket.h"
#include <fcntl.h>
#include <utility>

Acceptor::Acceptor(EventLoop *loop) {
  socket_ = std::make_unique<Socket>();
  assert(socket_->Create() == RC::RC_SUCCESS);
  assert(socket_->Bind("127.0.0.1", 1234) == RC::RC_SUCCESS);
  assert(socket_->Listen() == RC_SUCCESS);

  channel_ = std::make_unique std::function<void()> cb =
      std::bind(&Acceptor::AcceptConnection, this);
  acceptChannel->SetReadCallback(cb);
  acceptChannel->EnableRead();
}

Acceptor::~Acceptor() {}

void Acceptor::AcceptConnection() {
  InetAddress *clnt_addr = new InetAddress();
  Socket *clnt_sock = new Socket(sock_->Accept(clnt_addr));
  clnt_sock->SetNonBlocking(); // 新接受到的连接设置为非阻塞式
  if (new_connection_callback_) {
    new_connection_callback_(clnt_sock);
  }
  delete clnt_addr;
}

void Acceptor::SetNewConnectionCallback(
    std::function<void(Socket *)> const &callback) {
  new_connection_callback_ = callback;
}
