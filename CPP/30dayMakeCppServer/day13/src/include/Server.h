#pragma once
#include <map>
#include <vector>

class EventLoop;
class Socket;
class Acceptor;
class Connection;
class ThreadPool;
class Server {
 private:
  EventLoop *mainReactor;  // 只负责接受连接，然后分发给一个subReactor
  std::vector<EventLoop *> subReactors;    // 负责处理事件循环
  Acceptor *acceptor;                      // 连接接受器
  std::map<int, Connection *> connections;  // TCP连接
  ThreadPool *thpool;                       // 线程池

 public:
  explicit Server(EventLoop *);
  ~Server();

  /// @brief 处理可读事件
  void handleReadEvent(int);

  /// @brief 新建连接回调：分配连接到subReactor
  void newConnection(Socket *sock);

  /// @brief 连接关闭回调
  void deleteConnection(int sockfd);
};
