#pragma once
#include <sys/epoll.h>

class Epoll;
class Channel
{
private:
    Epoll *ep;          // 关联的Epoll实例指针     
    int fd;         
    uint32_t events;    // 关注的epoll事件类型
    uint32_t revents;   // 实际发生的epoll事件类型
    bool inEpoll;       // 是否已注册到epoll实例中
public:
    Channel(Epoll *_ep, int _fd);
    ~Channel();

    /// @brief 启用可读事件监听
    void enableReading();

    int getFd();
    uint32_t getEvents();
    uint32_t getRevents();
    bool getInEpoll();
    void setInEpoll();

    // void setEvents(uint32_t);
    
    /// @brief 设置内核通知的实际发生的epoll事件类型
    void setRevents(uint32_t);
};
