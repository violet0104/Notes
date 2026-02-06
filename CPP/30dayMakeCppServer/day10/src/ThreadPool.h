#pragma once
#include <functional>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>

class ThreadPool
{
private:
    std::vector<std::thread> threads;           // 工作线程
    std::queue<std::function<void()>> tasks;    // 任务队列
    std::mutex tasks_mtx;           // 互斥锁：用于线程互斥访问任务的队列
    std::condition_variable cv;     // 条件变量：实现工作线程的阻塞和唤醒
    bool stop;  // 线程停止标志
public:
    /// @brief 创建工作线程
    ThreadPool(int size = 10);
    ~ThreadPool();
    void add(std::function<void()>);
};