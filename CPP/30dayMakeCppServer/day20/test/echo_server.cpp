#include "pine.h"
#include <iostream>

int main() {
  TcpServer *server = new TcpServer("127.0.0.1", 1234);

  Signal::signal(SIGINT, [&] {
    std::cout << "\nServer exit!" << std::endl;
    exit(0);
  });

  server->OnConnect([](Connection *conn) {
    std::cout << "New connection fd: " << conn->GetSocket()->GetFd()
              << std::endl;
  });

  server->OnRecv([](Connection *conn) {
    // std::cout << "Message from client " << conn->ReadBuffer() << std::endl;
    if (conn->GetState() == Connection::State::Connected) {
      conn->Send(conn->GetReadBuffer()->ToStr());
    }
  });

  server->Start();

  delete server;
  return 0;
}
