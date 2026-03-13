#include "Connection.h"
#include "Socket.h"
#include <iostream>

int main() {
    Socket *sock = new Socket();
    sock->Connect("127.0.0.1", 1234);

    Connection *conn = new Connection(nullptr, sock);
    while (true) {
        conn->Read();
        std::cout << "Message from server: " << conn->ReadBuffer() << std::endl;
    }
    
    delete conn;
    return 0;
}