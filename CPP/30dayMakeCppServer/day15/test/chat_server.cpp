#include <iostream>
#include <map>

#include "Server.h"
#include "Socket.h"
#include "Connection.h"
#include "EventLoop.h"

int main() {
    std::map<int, Connection *> clients;
    EventLoop *loop = new EventLoop();
    Server *server = new Server(loop);

    server->NewConnect( [&clients](Connection *conn) {
        int clnt_fd = conn->GetSocket()->GetFd();
        std::cout << "New connection fd: " << clnt_fd << std::endl;
        clients[clnt_fd] = conn;
        for(auto &each : clients) {
            Connection *client = each.second;
            client->Send(conn->ReadBuffer());
        }
    });

    server->OnMessage( [&](Connection *conn) {
        std::cout << "Message from client " << conn->ReadBuffer() << std::endl;
        for(auto &each : clients){
            Connection *client = each.second;
            client->Send(conn->ReadBuffer());
        }
    });
}