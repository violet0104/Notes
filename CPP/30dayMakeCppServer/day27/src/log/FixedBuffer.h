#pragma once

#include <cstring>
#include <string>

// 固定大小的缓冲区，用于日志暂存
// SIZE 通常设置为 4MB (4000 * 1000)
template <int SIZE> class FixedBuffer {
public:
  FixedBuffer() : cur_(data_) {}

  ~FixedBuffer() = default;

  // 向缓冲区追加数据
  void append(const char *buf, size_t len) {
    if (static_cast<size_t>(avail()) > len) {
      memcpy(cur_, buf, len);
      cur_ += len;
    }
  }

  const char *data() const { return data_; }
  int length() const { return static_cast<int>(cur_ - data_); }

  // 写指针
  char *current() { return cur_; }
  int avail() const { return static_cast<int>(end() - cur_); }
  void add(size_t len) { cur_ += len; }

  // 重置缓冲区
  void reset() { cur_ = data_; }
  void bzero() { memset(data_, 0, sizeof(data_)); }

  // 转换为 string (主要用于调试)
  std::string toString() const { return std::string(data_, length()); }

private:
  const char *end() const { return data_ + sizeof(data_); }

  char data_[SIZE];
  char *cur_;
};
