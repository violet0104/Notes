#include "Acceptor.h"

#include "Channel.h"
#include "Macros.h"
#include "Socket.h"
#include <cassert>
#include <fcntl.h>
#include <utility>

Acceptor::Acceptor(EventLoop *loop, const char *ip, uint16_t port) {
  socket_ = std::unique_ptr<Socket>(new Socket());
  assert(socket_->Create() == RC::RC_SUCCESS);
  assert(socket_->SetReuseAddr() == RC::RC_SUCCESS);
  assert(socket_->Bind(ip, port) == RC::RC_SUCCESS);
  assert(socket_->Listen() == RC_SUCCESS);

  channel_ = std::unique_ptr<Channel>(new Channel(socket_->GetFd(), loop));
  std::function<void()> cb = std::bind(&Acceptor::AcceptConnection, this);
  channel_->SetReadCallback(cb);
  channel_->EnableRead();
}

Acceptor::~Acceptor() {}

RC Acceptor::AcceptConnection() const {
  int clnt_fd = -1;
  if (socket_->Accept(clnt_fd) != RC_SUCCESS) {
    return RC_ACCEPTOR_ERROR;
  }
  fcntl(clnt_fd, F_SETFL,
        fcntl(clnt_fd, F_GETFL) | O_NONBLOCK); // 新接受到的连接设置为非阻塞式
  if (new_connection_callback_) {
    new_connection_callback_(clnt_fd);
  }
  return RC_SUCCESS;
}

void Acceptor::SetNewConnectionCallback(
    std::function<void(int)> const &callback) {
  new_connection_callback_ = std::move(callback);
}
