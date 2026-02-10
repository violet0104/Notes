#pragma once
#include <sys/epoll.h>
#include <functional>

class EventLoop;
class Channel
{
private:
    EventLoop *loop;
    int fd;
    uint32_t events;    // fd关心的事件
    uint32_t ready;     // fd就绪的事件
    bool inEpoll;
    bool useThreadPool;
    std::function<void()> readCallback;
    std::function<void()> writeCallback;
public:
    Channel(EventLoop *_loop, int _fd);
    ~Channel();

    /// @brief 事件处理
    void handleEvent();

    /// @brief 开启读事件监听
    void enableReading();

    int getFd();

    /// @brief 获取关心的事件
    uint32_t getEvents();

    /// @brief 获取就绪的事件
    uint32_t getReady();

    /// @brief 判断fd是否已注册到epoll
    bool getInEpoll();

    /// @brief 标记fd已注册到epoll
    void setInEpoll(bool _in = true);

    /// @brief 开启 ET 模式 (默认使用LT)
    void useET();

    /// @brief EventLoop把epoll_wait返回的就绪事件设置到Channel中
    void setReady(uint32_t);

    /// @brief 设置读事件就绪后的回调函数
    void setReadCallback(std::function<void()>);

    /// @brief 设置是否使用线程池
    void setUseThreadPool(bool use = true);
};
