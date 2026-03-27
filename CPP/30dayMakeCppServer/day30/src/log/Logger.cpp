#include "Logger.h"
#include "AsyncLogging.h"
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <sys/time.h>
#include <thread>

Logger &Logger::Instance() {
  static Logger instance;
  return instance;
}

Logger::Logger() : level_(LogLevel::INFO) {
  // 初始化异步日志，默认文件名前缀为 "pine_server"，滚动大小为 100MB
  async_logging_ = new AsyncLogging("pine_server", 100 * 1024 * 1024);
}

Logger::~Logger() {
  if (async_logging_) {
    delete async_logging_;
  }
}

// 将日志级别转换为字符串
static const char *LogLevelToString(LogLevel level) {
  switch (level) {
  case LogLevel::TRACE:
    return "TRACE";
  case LogLevel::DEBUG:
    return "DEBUG";
  case LogLevel::INFO:
    return "INFO ";
  case LogLevel::WARN:
    return "WARN ";
  case LogLevel::ERROR:
    return "ERROR";
  case LogLevel::FATAL:
    return "FATAL";
  default:
    return "UNKNOWN";
  }
}

void Logger::Log(LogLevel level, const char *file, int line, const char *format,
                 ...) {
  // 提取文件名（去掉前面的路径）
  const char *file_name = strrchr(file, '/');
  if (file_name) {
    file_name++;
  } else {
    file_name = file;
  }

  // 获取当前时间
  struct timeval tv;
  gettimeofday(&tv, nullptr);
  struct tm *tm_time = localtime(&tv.tv_sec);
  char time_buf[32];
  strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", tm_time);

  // 获取当前线程对应的哈希值（简单替代线程号）
  auto thread_id = std::this_thread::get_id();

  // 解析可变参数
  char msg_buf[1024]; // 日志最大长度限制在 1024 字节内
  va_list args;
  va_start(args, format);
  vsnprintf(msg_buf, sizeof(msg_buf), format, args);
  va_end(args);

  // 格式化输出: [时间] [级别] [线程ID] [文件:行号] 消息内容
  char full_msg[2048];
  int len = snprintf(full_msg, sizeof(full_msg), "[%s.%06ld] [%s] [T:%zu] [%s:%d] %s\n",
                     time_buf, tv.tv_usec, LogLevelToString(level),
                     std::hash<std::thread::id>{}(thread_id), file_name, line,
                     msg_buf);

  // 如果异步日志已启动，写入异步日志
  if (async_logging_) {
    async_logging_->Append(full_msg, len);
  }

  // 对于 ERROR 和 FATAL 级别，额外输出到控制台以便即时反馈
  if (level >= LogLevel::ERROR) {
    fprintf(stderr, "%s", full_msg);
    fflush(stderr);
  }
}
