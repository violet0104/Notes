#ifndef CPP_30DAYMAKECPPSERVER_DAY13_SRC_INCLUDE_SOCKET_H_
#define CPP_30DAYMAKECPPSERVER_DAY13_SRC_INCLUDE_SOCKET_H_

class InetAddress;
class Socket {
private:
  int fd;

public:
  Socket();
  explicit Socket(int);
  ~Socket();

  void bind(InetAddress *);
  void listen();
  void setnonblocking();

  int accept(InetAddress *);
  void connect(InetAddress *);

  int getFd();
};

#endif // CPP_30DAYMAKECPPSERVER_DAY13_SRC_INCLUDE_SOCKET_H_
