#include "LogFile.h"
#include <cstdio>
#include <ctime>
#include <unistd.h>

LogFile::LogFile(const std::string &basename, off_t rollSize, int flushInterval)
    : basename_(basename), rollSize_(rollSize), flushInterval_(flushInterval),
      count_(0), startOfPeriod_(0), lastRoll_(0), lastFlush_(0), fp_(nullptr) {
  RollFile();
}

LogFile::~LogFile() {
  if (fp_) {
    fclose(fp_);
  }
}

void LogFile::Append(const char *logline, int len) {
  AppendInLock(logline, len);
}

void LogFile::Flush() {
  if (fp_) {
    fflush(fp_);
  }
}

void LogFile::AppendInLock(const char *logline, int len) {
  size_t n = fwrite(logline, 1, len, fp_);
  size_t remain = len - n;
  while (remain > 0) {
    size_t x = fwrite(logline + n, 1, remain, fp_);
    if (x == 0) {
      int err = ferror(fp_);
      if (err) {
        fprintf(stderr, "LogFile::AppendInLock failed %d\n", err);
      }
      break;
    }
    n += x;
    remain = len - n;
  }

  count_++;

  // 检查是否需要滚动文件
  if (ftell(fp_) >= rollSize_) {
    RollFile();
  } else {
    time_t now = time(nullptr);
    time_t thisPeriod = now / (24 * 3600) * (24 * 3600);
    if (thisPeriod != startOfPeriod_) {
      RollFile();
    } else if (now - lastFlush_ > flushInterval_) {
      lastFlush_ = now;
      Flush();
    }
  }
}

bool LogFile::RollFile() {
  time_t now = 0;
  std::string filename = GetLogFileName(basename_, &now);
  time_t start = now / (24 * 3600) * (24 * 3600);

  if (now > lastRoll_) {
    lastRoll_ = now;
    lastFlush_ = now;
    startOfPeriod_ = start;
    if (fp_) {
      fclose(fp_);
    }
    fp_ = fopen(filename.c_str(), "ae"); // 'e' for O_CLOEXEC
    return fp_ != nullptr;
  }
  return false;
}

std::string LogFile::GetLogFileName(const std::string &basename, time_t *now) {
  std::string filename;
  filename.reserve(basename.size() + 64);
  filename += basename;

  char timebuf[32];
  struct tm tm;
  *now = time(nullptr);
  localtime_r(now, &tm);
  strftime(timebuf, sizeof(timebuf), ".%Y%m%d-%H%M%S", &tm);
  filename += timebuf;

  char hostname[256];
  if (gethostname(hostname, sizeof(hostname)) == 0) {
    filename += ".";
    filename += hostname;
  }

  char pidbuf[32];
  snprintf(pidbuf, sizeof(pidbuf), ".%d", getpid());
  filename += pidbuf;

  filename += ".log";

  return filename;
}
