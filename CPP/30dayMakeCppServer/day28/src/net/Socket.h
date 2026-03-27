#pragma once
#include "Macros.h"
#include <cstdint>
#include <string>

class Socket {
public:
  DISALLOW_COPY_AND_MOVE(Socket);
  Socket();
  ~Socket();
  void SetFd(int fd);
  int GetFd() const;
  std::string GetAddr() const;
  RC Create();
  RC Bind(const char *ip, uint16_t port) const;
  RC Listen() const;
  RC Accept(int &clnt_fd) const;
  RC Connect(const char *ip, uint16_t port) const;
  RC SetNonBlocking() const;
  bool IsNonBlocking() const;
  size_t RecvBufSize() const;

private:
  int fd_;
};
