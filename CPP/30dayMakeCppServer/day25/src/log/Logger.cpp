#include "Logger.h"
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <sys/time.h>
#include <thread>

Logger &Logger::Instance() {
  static Logger instance;
  return instance;
}

Logger::Logger() : level_(LogLevel::INFO) {}

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
  // 使用互斥锁保证多线程环境下向控制台输出不被交错打断
  std::lock_guard<std::mutex> lock(mutex_);
  FILE *stream = (level >= LogLevel::ERROR) ? stderr : stdout;

  fprintf(stream, "[%s.%06ld] [%s] [T:%zu] [%s:%d] %s\n", time_buf, tv.tv_usec,
          LogLevelToString(level), std::hash<std::thread::id>{}(thread_id),
          file_name, line, msg_buf);
  fflush(stream);
}
