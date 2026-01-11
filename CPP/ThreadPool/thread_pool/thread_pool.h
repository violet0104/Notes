#ifndef ZDC_THREADPOOL_H
#define ZDC_THREADPOOL_H

#include <atomic>
#include <functional>
#include <future>
#include <thread>
#include <vector>
#include <queue>

namespace zdc
{

/// @brief C++11的线程池, 提交任务支持任意多参数, 支持获取返回值
class threadpool
{
    using task_t = std::function<void()>;   // 定义任务类型为可调用对象（无参， 无返回值）

    /// @brief 默认线程数 
    static constexpr std::size_t k_fallback_thread_count = 4;

    /// @brief 线程数的取值范围
    static constexpr std::size_t k_min_thread_count = 1;
    static constexpr std::size_t k_max_thread_count = 4096;

public:
    /// @brief 线程池当前的状态信息结构体
    std::size_t total_threads;  // 总线程数
    std::size_t busy_thread;    // 正在执行任务的线程数
    std::size_t idle_thread;    // 空闲线程数
    std::size_t pending_tasks;  // 等待的任务数
    bool running;               // 线程池是否在运行

    /// @brief 关闭线程池的模式
    enum class shutdown_mode : unsigned char
    {
        /// @brief 等待所有已提交的任务完成后再关闭线程池
        /// 在此模式下, 线程池会等待所有任务(包括已开始和未开始的任务)执行完成后再关闭.
        WaitForAllTasks,

        /// @brief 立即关闭线程池, 丢弃尚未开始的任务.
        /// 在此模式下, 线程池会立即停止接收新任务, 丢弃所有尚未开始执行的任务,
        /// 但已经开始执行的任务会继续执行, 直到它们完成.
        DiscardPendingTasks
    };

public:
    /// @brief 构造函数，初始化线程池
    /// @param thread_count 要创建的线程数量
    explicit threadpool(size_t thread_count  = default_thread_count()) {
        launch_threads(thread_count);
    }

    /// @brief 析构函数, 停止所有线程并等待它们完成
    ~threadpool() 
    {
        shutdown(shutdown_mode::WaitForAllTasks);
    }

    /// @brief 提交任务到线程池并返回一个 future 对象，用户可以通过它获取任务的返回值
    ///
    /// @tparam F 任务类型的可调用对象
    /// @tparam Args 可调用对象的参数类型
    /// @param f 需要提交的任务
    /// @param args 任务的参数
    /// @return std::future<decltype(f(args...))> 返回一个 future 对象, 允许用户获取任务的返回值
    template <typename F, typename...Args>
    auto submit(F &&f, Args &&...args) -> std::future<decltype(f(args...))>
    {

    }

    /// @brief 阻塞直到所有任务完成(任务队列为空且没有任务在执行), 若没有任务，立即返回
    void wait_all() {

    }

    /// @brief 关闭线程池
    /// @param mode `WaitForAllTasks` 等待所有任务执行完成后再关闭; `DiscardPendingTasks` 立即关闭线程池,
    /// 抛弃尚未开始的任务.
    void shutdown(shutdown_mode mode = shutdown_mode::WaitForAllTasks)
    {

    }

    /// @brief 重启线程池，先关闭当前线程池(等待所有任务完成), 然后以指定的线程数量重新启动线程池
    /// @param thread_count 要创建的工作线程数量
    void reboot(std::size_t thread_count)
    {

    }

private:
    static std::size_t default_thread_count()
    {
        auto n = std::thread::hardware_concurrency();
        return n == 0 ? k_fallback_thread_count : n;
    }

    /// @brief 启动线程池，创建指定数量的工作线程
    /// @param thread_count 线程池中线程的数量
    void launch_threads(std::size_t thread_count) 
    {

    }

private:
    std::vector<std::thread> workers_;  // 工作线程集合，用于并发执行任务
    std::queue<task_t> task_queue_;     // 等待执行的任务队列
    std::condition_variable cv_;        // 条件变量，用于通知工作线程有新的任务到来
    mutable std::mutex mtx_;            // 主互斥锁，保护任务队列和与其相关的状态（加 mutable 是因为const函数可能也要访问互斥锁）           

    std::atomic<std::size_t> busy_count_{0};    // 正在执行任务的线程数量
    std::atomic<bool> running_{true};           // 线程池是否处于运行状态

    mutable std::mutex mtx_done_;       // 用于保护完成通知的互斥锁(wait_all 用)
    std::condition_variable cv_done_;   // 条件变量，用于等待所有任务执行完毕(配合 wait_all 使用)
};

}

#endif