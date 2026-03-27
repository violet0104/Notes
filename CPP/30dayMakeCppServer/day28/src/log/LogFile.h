#pragma once

#include "Macros.h"
#include <string>

// 低级别的文件写入封装，不包含缓冲区，主要负责 fwrite 和周期性 flush
class LogFile {
public:
  LogFile(const std::string &basename, off_t rollSize, int flushInterval = 3);
  ~LogFile();

  DISALLOW_COPY_AND_MOVE(LogFile);

  /// @brief 向日志文件中追加数据
  void Append(const char *logline, int len);

  /// @brief 强制将缓冲区数据刷向磁盘
  void Flush();

  /// @brief 日志文件滚动
  bool RollFile();

private:
  void AppendInLock(const char *logline, int len);

  // 获取日志文件完整名称
  static std::string GetLogFileName(const std::string &basename, time_t *now);

  const std::string basename_; // 日志文件名
  const off_t rollSize_; // 滚动阈值
  const int flushInterval_; // 刷新间隔

  int count_;            // 计数器
  time_t startOfPeriod_; // 周期开始时间
  time_t lastRoll_;      // 上次滚动时间
  time_t lastFlush_;     // 上次刷盘时间
  FILE *fp_;             // 文件指针
};
