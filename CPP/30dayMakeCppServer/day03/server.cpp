#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <errno.h>
#include "util.h"

#define MAX_EVENTS 1024
#define READ_BUFFER 1024

void setnonblocking(int fd) {
    fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);
}

int main() {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    errif(sockfd == -1, "socket create error");

    struct sockaddr_in serv_addr;
    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serv_addr.sin_port = htons(8888);

    errif(bind(sockfd, (sockaddr *)&serv_addr, sizeof(serv_addr)) == -1, "socket bind error");
    
    errif(listen(sockfd, SOMAXCONN) == -1, "socket listen error");

    // 创建 epoll 实例，即创建一个事件表
    int epfd = epoll_create1(0);
    errif(epfd == -1, "eproll create error");

    // events[MAX_EVENTS]：定义数组，存储 epoll_wait 返回的就绪事件
    // ev：定义单个事件结构体，用于配置要添加到 epoll 的事件
    struct epoll_event events[MAX_EVENTS], ev;
    bzero(&events, sizeof(events));
    
    bzero(&ev, sizeof(ev));
    ev.data.fd = sockfd;        // 该 IO 口为服务器 socket fd
    ev.events = EPOLLIN;        // ET 模式
    setnonblocking(sockfd);
    epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd, &ev);    // 将服务器socket fd添加到epoll

    // 不断监听epoll上的事件并处理
    while (true) {
        // 有nfds个fd发生事件
        int nfds = epoll_wait(epfd, events, MAX_EVENTS, -1);
        // 处理这nfds个事件
        for (int i = 0; i < nfds; ++i) {
            // 发生事件的fd是服务器socket fd，表示有新客户端连接
            if (events[i].data.fd == sockfd) {
                struct sockaddr_in clnt_addr;
                socklen_t clnt_addr_len = sizeof(clnt_addr);
                bzero(&clnt_addr, sizeof(clnt_addr));

                int clnt_sockfd = accept(sockfd, (sockaddr*)&clnt_addr, &clnt_addr_len);
                errif(clnt_sockfd == -1, "socket accept error");

                printf("new client fd %d! IP: %s Port: %d\n", clnt_sockfd, inet_ntoa(clnt_addr.sin_addr), ntohs(clnt_addr.sin_port));
                
                // 重新初始化事件ev
                bzero(&ev, sizeof(ev));
                ev.data.fd = clnt_sockfd;
                // 对于客户端连接，使用ET模式，可以让epoll更加高效，支持更多并发
                ev.events = EPOLLIN | EPOLLET;
                // ET需要搭配非阻塞式socket使用
                setnonblocking(clnt_sockfd);
                // 将客户端socket fd添加到epoll
                epoll_ctl(epfd, EPOLL_CTL_ADD, clnt_sockfd, &ev);
            } else if (events[i].events & EPOLLIN) {  // 发生事件的是客户端，并且是可读事件（EPOLLIN）
                char buf[READ_BUFFER];
                // 由于使用非阻塞IO，读取客户端buffer，一次读取buf大小数据，直到全部读取完毕
                while (true) {
                    bzero(&buf, sizeof(buf));
                    ssize_t read_bytes = read(events[i].data.fd, buf, sizeof(buf));
                    if (read_bytes > 0) {
                        printf("message from client fd %d: %s\n", events[i].data.fd, buf);
                        write(events[i].data.fd, buf, sizeof(buf));
                    } else if (read_bytes == -1 && errno == EINTR) {    //客户端正常中断、继续读取
                        printf("continue reading");
                        continue;
                    } else if (read_bytes == -1 && ((errno == EAGAIN) || errno == EWOULDBLOCK)) { // 非阻塞IO，这个条件表示数据全部读取完毕
                        printf("finish reading once, errno: %d\n", errno);
                        break;
                    } else if (read_bytes == 0) {   // EOF，客户端断开连接
                        printf("EOF, client fd %d disconnected\n", events[i].data.fd);
                        close(events[i].data.fd);   //关闭socket会自动将文件描述符从epoll树上移除
                        break;
                    }
                }
            } else {    // 其他事件，之后的版本实现
                printf("something else happened\n");
            }
        }
    }
    close(sockfd);
    return 0;
}