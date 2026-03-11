#pragma once
#include <map>
#include <vector>
#include <functional>
#include "Macros.h"

class EventLoop;
class Socket;
class Acceptor;
class Connection;
class ThreadPool;
class Server {
private:
  EventLoop *main_reactor_; // 只负责接受连接，然后分发给一个subReactor
  std::vector<EventLoop *> sub_reactors_;    // 负责处理事件循环
  Acceptor *acceptor_;                      // 连接接受器
  std::map<int, Connection *> connections_; // TCP连接
  ThreadPool *thread_pool_;                      // 线程池
  std::function<void(Connection *)> on_connec_callback_;

public:
  explicit Server(EventLoop *);
  ~Server();

  DISALLOW_COPY_AND_MOVE(Server);

  /// @brief 新建连接回调：分配连接到subReactor
  void NewConnection(Socket *sock);

  /// @brief 连接关闭回调
  void DeleteConnection(Socket *sock);

  /// @brief 用户自定义业务逻辑
  void OnConnect(std::function<void(Connection *)> fn);
};
