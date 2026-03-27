#include "Logger.h"
#include <thread>
#include <vector>

void worker(int id) {
    LOG_INFO("Worker %d starting", id);
    for (int i = 0; i < 5; ++i) {
        LOG_DEBUG("Worker %d doing task %d", id, i);
    }
    LOG_INFO("Worker %d finished", id);
}

int main() {
    // 设置全局日志级别为 DEBUG 及以上
    Logger::Instance().SetLevel(LogLevel::DEBUG);

    LOG_TRACE("This is a trace log (should not show up if level is DEBUG)");
    LOG_DEBUG("Logger test started.");

    std::vector<std::thread> threads;
    for (int i = 0; i < 3; ++i) {
        threads.emplace_back(worker, i);
    }

    for (auto &t : threads) {
        t.join();
    }

    LOG_WARN("Work is done, closing down.");
    LOG_ERROR("Simulated an error just to see the ERROR log format.");

    return 0;
}
