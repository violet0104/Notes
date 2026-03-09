#ifndef CPP_30DAYMAKECPPSERVER_DAY13_SRC_INCLUDE_INETADDRESS_H_
#define CPP_30DAYMAKECPPSERVER_DAY13_SRC_INCLUDE_INETADDRESS_H_

#include <arpa/inet.h>

class InetAddress {
 public:
  struct sockaddr_in addr;
  socklen_t addr_len;
  InetAddress();
  InetAddress(const char *ip, uint16_t port);
  ~InetAddress();

  void setInetAddr(sockaddr_in _addr, socklen_t _addr_len);
  sockaddr_in getAddr();
  socklen_t getAddr_len();
};

#endif  // CPP_30DAYMAKECPPSERVER_DAY13_SRC_INCLUDE_INETADDRESS_H_
