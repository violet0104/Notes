#pragma once

#include <cstdlib>
#include <mutex>

// 日志级别
enum class LogLevel { TRACE, DEBUG, INFO, WARN, ERROR, FATAL };

class AsyncLogging;

class Logger {
public:
  // 获取单例实例
  static Logger &Instance();

  // 设置全局日志最低级别
  void SetLevel(LogLevel level) { level_ = level; }

  // 获取当前日志级别
  LogLevel GetLevel() const { return level_; }

  // 日志核心打印函数
  void Log(LogLevel level, const char *file, int line, const char *format, ...);

private:
  Logger();
  ~Logger();

  Logger(const Logger &) = delete;
  Logger &operator=(const Logger &) = delete;

private:
  LogLevel level_;
  std::mutex mutex_; // 保护向控制台输出的线程安全（如果启用了控制台打印）
  AsyncLogging *async_logging_{nullptr}; // 异步日志组件
};

// ==========================================
// 日志宏定义
// ==========================================

#define LOG_TRACE(format, ...)                                                 \
  do {                                                                         \
    if (Logger::Instance().GetLevel() <= LogLevel::TRACE)                      \
      Logger::Instance().Log(LogLevel::TRACE, __FILE__, __LINE__, format,      \
                             ##__VA_ARGS__);                                   \
  } while (0)

#define LOG_DEBUG(format, ...)                                                 \
  do {                                                                         \
    if (Logger::Instance().GetLevel() <= LogLevel::DEBUG)                      \
      Logger::Instance().Log(LogLevel::DEBUG, __FILE__, __LINE__, format,      \
                             ##__VA_ARGS__);                                   \
  } while (0)

#define LOG_INFO(format, ...)                                                  \
  do {                                                                         \
    if (Logger::Instance().GetLevel() <= LogLevel::INFO)                       \
      Logger::Instance().Log(LogLevel::INFO, __FILE__, __LINE__, format,       \
                             ##__VA_ARGS__);                                   \
  } while (0)

#define LOG_WARN(format, ...)                                                  \
  do {                                                                         \
    if (Logger::Instance().GetLevel() <= LogLevel::WARN)                       \
      Logger::Instance().Log(LogLevel::WARN, __FILE__, __LINE__, format,       \
                             ##__VA_ARGS__);                                   \
  } while (0)

#define LOG_ERROR(format, ...)                                                 \
  do {                                                                         \
    if (Logger::Instance().GetLevel() <= LogLevel::ERROR)                      \
      Logger::Instance().Log(LogLevel::ERROR, __FILE__, __LINE__, format,      \
                             ##__VA_ARGS__);                                   \
  } while (0)

#define LOG_FATAL(format, ...)                                                 \
  do {                                                                         \
    Logger::Instance().Log(LogLevel::FATAL, __FILE__, __LINE__, format,        \
                           ##__VA_ARGS__);                                     \
    exit(EXIT_FAILURE);                                                        \
  } while (0)
