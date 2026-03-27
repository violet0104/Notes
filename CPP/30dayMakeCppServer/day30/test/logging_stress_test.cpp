#include "Logger.h"
#include <chrono>
#include <iostream>
#include <thread>
#include <vector>
#include <atomic>

// 压力测试：多个线程同时高速写日志
void StressTest(int thread_count, int logs_per_thread) {
    std::vector<std::thread> threads;
    std::atomic<int> total_logs(0);
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < thread_count; ++i) {
        threads.emplace_back([i, logs_per_thread, &total_logs]() {
            for (int j = 0; j < logs_per_thread; ++j) {
                LOG_INFO("Thread %d logging message %d: Testing async logging performance and stability under heavy load.", i, j);
                total_logs++;
            }
        });
    }
    
    for (auto &t : threads) {
        t.join();
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "Stress test finished.\n";
    std::cout << "Threads: " << thread_count << "\n";
    std::cout << "Logs per thread: " << logs_per_thread << "\n";
    std::cout << "Total logs: " << total_logs << "\n";
    std::cout << "Time taken: " << diff.count() << " ms\n";
    std::cout << "Average throughput: " << (total_logs * 1000.0 / diff.count()) << " logs/sec\n";
}

int main() {
    // 设置日志级别
    Logger::Instance().SetLevel(LogLevel::INFO);
    
    std::cout << "Starting logging stress test...\n";
    
    // 模拟 10 个线程，每个写 10 万条日志
    StressTest(10, 100000);
    
    std::cout << "Test completed. Check the log files for integrity.\n";
    
    return 0;
}
