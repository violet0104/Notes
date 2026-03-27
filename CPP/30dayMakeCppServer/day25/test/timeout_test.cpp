#include "Socket.h"
#include "TcpServer.h"
#include <iostream>
#include <unistd.h>
#include <thread>
#include <cassert>

int main() {
  std::thread server_thread([](){
    TcpServer server("127.0.0.1", 12345);
    server.Start();
  });
  server_thread.detach();

  // 等待服务器启动
  sleep(1);

  Socket client;
  client.Create();
  RC rc = client.Connect("127.0.0.1", 12345);
  if (rc != RC_SUCCESS) {
    std::cout << "Test failed! Cannot connect." << std::endl;
    return -1;
  }

  std::cout << "Client connected. Waiting for 12 seconds to trigger timeout..." << std::endl;
  sleep(12); // 等超过 10s

  char buf[1024];
  int n = read(client.GetFd(), buf, sizeof(buf));
  if (n == 0) {
    std::cout << "Server closed connection as expected due to timeout." << std::endl;
  } else {
    std::cout << "Test failed! Expected connection to be closed, but got n=" << n << std::endl;
    assert(false);
  }

  std::cout << "timeout_test passed." << std::endl;
  return 0;
}
