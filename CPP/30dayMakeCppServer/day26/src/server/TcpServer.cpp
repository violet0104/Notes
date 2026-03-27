#include "TcpServer.h"
#include "Acceptor.h"
#include "Connection.h"
#include "EventLoop.h"
#include "ThreadPool.h"
#include <cassert>
#include <chrono>
#include <functional>
#include <memory>
#include "Logger.h"

TcpServer::TcpServer(const char *ip, uint16_t port) {
  main_reactor_ = std::unique_ptr<EventLoop>(new EventLoop());
  acceptor_ =
      std::unique_ptr<Acceptor>(new Acceptor(main_reactor_.get(), ip, port));
  std::function<void(int)> cb =
      std::bind(&TcpServer::NewConnection, this, std::placeholders::_1);
  acceptor_->SetNewConnectionCallback(cb);

  unsigned int size = std::thread::hardware_concurrency();
  thread_pool_ = std::unique_ptr<ThreadPool>(new ThreadPool(size));

  for (size_t i = 0; i < size; ++i) {
    std::unique_ptr<EventLoop> sub_reactor(new EventLoop());
    sub_reactors_.push_back(std::move(sub_reactor));
  }
}

TcpServer::~TcpServer() {}

void TcpServer::Start() {
  for (size_t i = 0; i < sub_reactors_.size(); ++i) {
    std::function<void()> sub_loop =
        std::bind(&EventLoop::Loop, sub_reactors_[i].get());
    thread_pool_->Add(std::move(sub_loop));
  }
  main_reactor_->Loop();
}

void TcpServer::Stop() {
  main_reactor_->Quit();
  for (auto &sub_reactor : sub_reactors_) {
    sub_reactor->Quit();
  }
}

RC TcpServer::NewConnection(int fd) {
  assert(fd != -1);
  uint64_t random = fd % sub_reactors_.size();

  std::unique_ptr<Connection> conn = std::unique_ptr<Connection>(
      new Connection(fd, sub_reactors_[random].get()));
  std::function<void(int)> cb =
      std::bind(&TcpServer::DeleteConnection, this, std::placeholders::_1);

  conn->SetDeleteConnectionCallback(cb);
  conn->SetOnRecvCallback(on_recv_);

  connections_[fd] = std::move(conn);
  if (on_connect_) {
    on_connect_(connections_[fd].get());
  }

  EventLoop *sub_loop = sub_reactors_[random].get();
  int timeout_interval = 10000;
  sub_loop->AddTimer(timeout_interval, [this, fd, sub_loop]() {
    this->CheckConnectionTimeout(fd, sub_loop);
  });

  return RC_SUCCESS;
}

RC TcpServer::DeleteConnection(int fd) {
  auto it = connections_.find(fd);
  assert(it != connections_.end());
  connections_.erase(fd);
  return RC_SUCCESS;
}

void TcpServer::OnConnect(std::function<void(Connection *)> fn) {
  on_connect_ = std::move(fn);
}

void TcpServer::OnRecv(std::function<void(Connection *)> fn) {
  on_recv_ = std::move(fn);
}

void TcpServer::CheckConnectionTimeout(int fd, EventLoop *sub_loop) {
  auto it = connections_.find(fd);
  if (it != connections_.end()) {
    Connection *conn = it->second.get();
    auto now = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        now - conn->GetLastActiveTime());
    int timeout_ms = 10000;
    if (duration.count() >= timeout_ms) {
      LOG_INFO("Connection %d timeout", fd);
      conn->Close();
    } else {
      sub_loop->AddTimer(timeout_ms - duration.count(), [this, fd, sub_loop]() {
        this->CheckConnectionTimeout(fd, sub_loop);
      });
    }
  }
}
