#pragma once
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
