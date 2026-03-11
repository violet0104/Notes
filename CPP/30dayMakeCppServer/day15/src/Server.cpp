#include "Server.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <functional>
#include <thread>
#include "Acceptor.h"
#include "Connection.h"
#include "EventLoop.h"
#include "Socket.h"
#include "ThreadPool.h"
#include "util.h"

Server::Server(EventLoop *loop) : main_reactor_(loop), acceptor_(nullptr) {
  acceptor_ = new Acceptor(main_reactor_); // Acceptor由且只由mainReactor负责
  std::function<void(Socket *)> cb = std::bind(&Server::NewConnection, this, std::placeholders::_1);
  acceptor_->SetNewConnectionCallback(cb);

  int size = static_cast<int>(std::thread::hardware_concurrency());
  thread_pool_ = new ThreadPool(size);
  for (int i = 0; i < size; ++i) {
    sub_reactors_.push_back(new EventLoop()); // 每一个线程是一个EventLoop
  }

  for (int i = 0; i < size; ++i) {
    std::function<void()> sub_loop = std::bind(&EventLoop::Loop, sub_reactors_[i]);
    thread_pool_->add(sub_loop); // 开启所有线程的事件循环
  }
}

Server::~Server() {
  delete acceptor_;
  delete thread_pool_;
}

void Server::NewConnection(Socket *sock) {
  ErrorIf(sock->GetFd() == -1, "new connection error");
  uint64_t random = sock->GetFd() % sub_reactors_.size();
  Connection *conn = new Connection(sub_reactors_[random], sock);
  std::function<void(Socket *)> cb = std::bind(&Server::DeleteConnection, this, std::placeholders::_1);
  conn->SetDeleteConnectionCallback(cb);
  conn->SetOnConnectCallback(on_connec_callback_);
  connections_[sock->GetFd()] = conn;
}

void Server::DeleteConnection(Socket *sock) {
  int sockfd = sock->GetFd();
  auto it = connections_.find(sockfd);
  if (it != connections_.end()) {
    Connection *conn = connections_[sockfd];
    connections_.erase(sockfd);
    delete conn;
    conn = nullptr;
  }
}

void Server::OnConnect(std::function<void(Connection *)> fn) {
  on_connec_callback_ = fn;
}
