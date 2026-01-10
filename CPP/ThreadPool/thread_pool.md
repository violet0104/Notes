# **C++ 线程池**



## **一、线程池**

预先创建一定数量的线程，将这些线程复用，从而避免了频繁地线程创建和销毁，减少了系统开销，提高可任务的执行效率。



## **二、C++ 线程池实现原理**

**核心部分**：

- 线程队列
- 任务队列
- 互斥锁
- 条件变量等



### 2.1 线程队列

**线程队列：**包含预先创建好的多个线程，这些线程在创建后不会立即执行任务，而是进入等待状态，随时准备接受任务的分配。

在C++中，我们可以利用`std::vector<std::thread>`来创建和管理线程队列。

```c++
std::vector<std::thread> workers_;
for (size_t i = 0; i < thread_count; ++i) {
    workers_.emplace_back([this] {this->work(); });
}
```

thead_count表示我们希望创建的线程数量，通过循环创建thread_count个线程，并把他们添加到threads线程队列中。每个线程都执行work函数，这个函数就是线程的工作逻辑所在。



### 2.2 任务队列

**任务队列：**存储执行任务的地方，当有新的任务到来时，就会被添加到这个队列中等待处理。

在C++中，可以用`std::queue`来实现任务队列，为了确保在多线程环境下的安全访问，还需要配合互斥锁和条件变量。

```c++
using task_t = std::function<void()>;	// 定义task_t为function<void()>：可以调用且无参数、无返回值的函数对象
std::queue<task_t> task_queue_;
mutable std::mutex mtx_;		// 互斥锁
std::condition_variable cv_;	// 条件变量
```



### 2.3 互斥锁

**互斥锁：**用来保护共享资源（任务队列等），确保同一时间只有一个线程能够访问任务队列。当一个线程想要访问任务队列（比如添加任务或取出任务）时，它必须先获取互斥锁。如果此时互斥锁已经被其他线程持有，那么这个线程就会被阻塞，直到互斥锁被释放。

在C++中，可以用`std::mutex`来实现互斥锁。推荐使用`std::lock_guard`或`std::unique_lock`，它们会在作用域结束时自动释放锁。

```c++
mutable std::mutex mtx_;

{
    std::lock_guard<std::mutex> lock(mtx_);
 // std::unique_lock<std::mutex> lock(mtx_);
    // 访问任务队列的代码
}	// {}作用域结束后，lock自动析构，释放锁
```

`lock_guard`不支持手动解锁，`unique_lock`支持手动解锁。



### 2.4 条件变量

**条件变量：**主要用于线程间的同步和通信。它与互斥锁配合使用，当任务队列中没有任务时，工作线程可以通过条件变量进入等待状态，释放互斥锁，让出 CPU 资源。当有新任务添加到任务队列时，就可以通过条件变量通知等待的线程，让它们醒来并获取互斥锁，从任务队列中取出任务执行。

在C++中，可以用`std::condition_variable`来实现条件变量。

```c++
std::condition_variable cv_;
{
	std::unique_lock<std::mutex> lock(mtx_);
	// 等待直到任务队列中有任务, 或者线程池已停止
	cv_.wait(lock, [this] { return !running_ || !task_queue_.empty(); });
	if (!running_ && task_queue_.empty()) return;  // 如果线程池已经停止并且队列为空, 退出线程
	task_t task = std::move(task_queue_.front());         // 从队列中取出任务
	task_queue_.pop();
}
```

在这段代码中：

1. `cv_.wait()`会使线程进入等待状态，并释放lock锁。
2. 当其他线程调用`cv_.notify_one()`或`cv_.notify_all()`通知时，等待的线程会被唤醒，重新获取lock锁。然后判断`!running_ || !task_queue_.empty()`，如果为false，重复步骤1；如果为true，则继续执行后续代码，从任务队列中取出任务。