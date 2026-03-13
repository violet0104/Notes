#include "Socket.h"

#include <fcntl.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <assert.h>
#include <sys/socket.h>

Socket::Socket() : fd_(-1) {}

Socket::~Socket() {
  if (fd_ != -1) {
    close(fd_);
    fd_ = -1;
  }
}

void Socket::set_fd(int fd) { fd_ = fd; }

int Socket::get_fd() const { return fd_; }

std::string Socket::get_addr() const {
  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  socklen_t len = sizeof(addr);
  if (getpeername(fd_, (struct sockaddr*)&addr, &len) == -1) {
    return "";
  }
  std::string ret(inet_ntoa(addr.sin_addr));
  ret += ":";
  ret += std::to_string(htons(addr.sin_port));
  return ret;
}

RC Socket::SetNonBlocking() const {
  if (fcntl(fd_, F_SETFL, fcntl(fd_, F_GETFL) | O_NONBLOCK) == -1) {
    perror("Socket set non-blocking failed");
    return RC_SOCKET_ERROR;
  }
  return RC_SUCCESS;
}

bool Socket::IsNonBlocking() const { return (fcntl(fd_, F_GETFL) & O_NONBLOCK) != 0; }

size_t Socket::RecvBufSize() const {
  size_t size = -1;
  if (ioctl(fd_, FIONREAD, &size) == -1) {
    perror("Socket get recv buf size failed");
  }
  return size;
}

RC Socket::Create() {
  assert(fd_ == -1);
  fd_ = socket(AF_INET, SOCK_STREAM, 0);
  if (fd_ == -1) {
    perror("Failed to create socket");
    return RC_SOCKET_ERROR;
  }
  return RC_SUCCESS;
}

int Socket::Accept(InetAddress *addr) {
  // for server socket
  int clnt_sockfd = -1;
  struct sockaddr_in tmp_addr {};
  socklen_t addr_len = sizeof(tmp_addr);
  if (IsNonBlocking()) {  // 阻塞
    while (true) {
      clnt_sockfd = ::accept(fd_, (sockaddr *)&tmp_addr, &addr_len);
      if (clnt_sockfd == -1 && ((errno == EAGAIN) || (errno == EWOULDBLOCK))) {
        // printf("no connection yet\n");
        continue;
      }
      if (clnt_sockfd == -1) {
        ErrorIf(true, "socket accept error");
      } else {
        break;
      }
    }
  } else {  // 非阻塞
    clnt_sockfd = ::accept(fd_, (sockaddr *)&tmp_addr, &addr_len);
    ErrorIf(clnt_sockfd == -1, "socket accept error");
  }
  addr->SetAddr(tmp_addr);
  return clnt_sockfd;
}

void Socket::Connect(InetAddress *addr) {
  // for client socket
  struct sockaddr_in tmp_addr = addr->GetAddr();
  if (IsNonBlocking()) {  // 阻塞
    while (true) {
      int ret = connect(fd_, (sockaddr *)&tmp_addr, sizeof(tmp_addr));
      if (ret == 0) {
        break;
      }
      if (ret == -1 && (errno == EINPROGRESS)) {  // 连接正在后台建立中（假失败）
        continue;
        /* 连接非阻塞式sockfd建议的做法：
            The socket is nonblocking and the connection cannot be
          completed immediately.  (UNIX domain sockets failed with
          EAGAIN instead.)  It is possible to select(2) or poll(2)
          for completion by selecting the socket for writing.  After
          select(2) indicates writability, use getsockopt(2) to read
          the SO_ERROR option at level SOL_SOCKET to determine
          whether connect() completed successfully (SO_ERROR is
          zero) or unsuccessfully (SO_ERROR is one of the usual
          error codes listed here, explaining the reason for the
          failure).
          这里为了简单、不断连接直到连接完成，相当于阻塞式
        */
      }
      if (ret == -1) {
        ErrorIf(true, "socket connect error");
      }
    }
  } else {  // 非阻塞
    ErrorIf(connect(fd_, (sockaddr *)&tmp_addr, sizeof(tmp_addr)) == -1, "socket connect error");
  }
}

void Socket::Connect(const char *ip, uint16_t port) {
  InetAddress *addr = new InetAddress(ip, port);
  Connect(addr);
  delete addr;
}


int Socket::GetFd() { return fd_; }

InetAddress::InetAddress() = default;
InetAddress::InetAddress(const char *ip, uint16_t port) {
  memset(&addr_, 0, sizeof(addr_));
  addr_.sin_family = AF_INET;
  addr_.sin_addr.s_addr = inet_addr(ip);
  addr_.sin_port = htons(port);
}

void InetAddress::SetAddr(sockaddr_in addr) { addr_ = addr; }

sockaddr_in InetAddress::GetAddr() { return addr_; }

const char *InetAddress::GetIp() { return inet_ntoa(addr_.sin_addr); }

uint16_t InetAddress::GetPort() { return ntohs(addr_.sin_port); }
