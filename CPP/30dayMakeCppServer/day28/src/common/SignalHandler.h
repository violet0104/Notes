#pragma once
#include <cstring>
#include <functional>
#include <map>
#include <signal.h>

struct Signal {
  // 使用静态方法获取单例 map，确保 C++11 下的多线程安全和单次定义
  static std::map<int, std::function<void()>> &GetHandlers() {
    static std::map<int, std::function<void()>> handlers;
    return handlers;
  }

  // 静态 C 风格信号处理函数
  static void signal_handler(int sig) {
    auto &handlers = GetHandlers();
    if (handlers.count(sig)) {
      handlers[sig]();
    }
  }

  static void signal(int sig, const std::function<void()> &handler) {
    GetHandlers()[sig] = handler;

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    if (sigaction(sig, &sa, nullptr) == -1) {
      // 错误处理可以在此处通过 LOG 记录
    }
  }
};