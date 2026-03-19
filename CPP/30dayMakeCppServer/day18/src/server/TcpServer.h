#pragma once
#include "Macros.h"
#include <functional>
#include <memory>
#include <unordered_map>
#include <vector>

class TcpServer {
public:
  DISALLOW_COPY_AND_MOVE(TcpServer);
  TcpServer();
  ~TcpServer();
  void Start();

  /// @brief 新建连接回调：分配连接到subReactor
  RC NewConnection(int fd);

  /// @brief 连接关闭回调
  RC DeleteConnection(int fd);

  void OnConnect(std::function<void(Connection *)> fn);
  void OnRecv(std::function<void(Connection *)> fn);

private:
  std::unique_ptr<EventLoop>
      main_reactor_; // 只负责接受连接，然后分发给一个subReactor
  std::vector<std::unique_ptr<EventLoop>> sub_reactors_; // 负责处理事件循环
  std::unique_ptr<Acceptor> acceptor_;                   // 连接接受器
  std::unordered_map<int, std::unique_ptr<Connection>> connections_; // TCP连接
  std::unique_ptr<ThreadPool> thread_pool_; // 线程池
  std::function<void(Connection *)> on_connect_;
  std::function<void(Connection *)> on_recv_;
};
