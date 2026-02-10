#include "Connection.h"
#include "Channel.h"
#include "Socket.h"
#include "Buffer.h"
#include <string.h>
#include <unistd.h>
#include <iostream>
#include "util.h"

Connection::Connection(EventLoop *_loop, Socket *_sock) : loop(_loop), sock(_sock) {
    channel = new Channel(loop, sock->getFd());
    channel->enableReading();
    channel->useET();
    std::function<void()> cb = std::bind(&Connection::echo, this, sock->getFd());
    channel->setReadCallback(cb);
    channel->setUseThreadPool(true);
    readBuffer = new Buffer();
}

Connection::~Connection() {
    delete channel;
    delete sock;
    delete readBuffer;
}

void Connection::setDeleteConnectionCallback(std::function<void(Socket *)> _cb){
    deleteConnectionCallback = _cb;
}

void Connection::echo(int sockfd) {
    char buf[1024];      // 这个buf大小无所谓
    // 由于使用非阻塞IO，读取客户端buffer，一次读取buf大小数据，直到全部读取完毕
    while (true) {
        bzero(&buf, sizeof(buf));
        ssize_t read_bytes = read(sockfd, buf, sizeof(buf));
        if (read_bytes > 0) {
            readBuffer->append(buf, read_bytes);
        } else if (read_bytes == -1 && errno == EINTR) {    //客户端正常中断、继续读取
            printf("continue reading");
            continue;
        } else if (read_bytes == -1 && ((errno == EAGAIN) || errno == EWOULDBLOCK)) { // 非阻塞IO，这个条件表示数据全部读取完毕
            printf("finish reading once, errno: %d\n", errno);
            printf("message from client fd %d: %s\n", sockfd, readBuffer->c_str());
            errif(write(sockfd, readBuffer->c_str(), readBuffer->size()) == -1, "socket write error");
            readBuffer->clear();
            break;
        } else if (read_bytes == 0) {   // EOF，客户端断开连接
            printf("EOF, client fd %d disconnected\n", sockfd);
            // close(sockfd);   //关闭socket会自动将文件描述符从epoll树上移除
            deleteConnectionCallback(sock);
            break;
        }
    }
}

void Connection::send(int sockfd) {
    char buf[readBuffer->size()];
    strcpy(buf, readBuffer->c_str());
    int data_size = readBuffer->size();
    int data_left = data_size;
    while (data_left > 0) {
        ssize_t bytes_write = write(sockfd, buf + data_size - data_left, data_left);
        if (bytes_write == -1 && errno == EAGAIN) { 
            break;
        }
        data_left -= bytes_write;
    }
    
}