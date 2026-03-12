#include "Acceptor.h"
#include "Channel.h"
#include "Socket.h"
#include <stdio.h>

Acceptor::Acceptor(EventLoop *loop) : loop_(loop) {
  sock_ = new Socket();
  InetAddress *addr = new InetAddress("127.0.0.1", 1234);
  sock_->Bind(addr);
  sock_->Listen();
  // sock->setnonblocking();  acceptor使用阻塞式IO比较好
  Channel *acceptChannel = new Channel(loop_, sock_);
  std::function<void()> cb = std::bind(&Acceptor::AcceptConnection, this);
  acceptChannel->SetReadCallback(cb);
  acceptChannel->EnableRead();
  delete addr;
}

Acceptor::~Acceptor() {
  delete sock_;
  delete channel_;
}

void Acceptor::AcceptConnection() {
  InetAddress *clnt_addr = new InetAddress();
  Socket *clnt_sock = new Socket(sock_->Accept(clnt_addr));
  clnt_sock->SetNonBlocking();  // 新接受到的连接设置为非阻塞式
  if(new_connection_callback_){
    new_connection_callback_(clnt_sock);
  }
  delete clnt_addr;
}

void Acceptor::SetNewConnectionCallback(std::function<void(Socket *)> const &callback) {
  new_connection_callback_ = callback;
}
