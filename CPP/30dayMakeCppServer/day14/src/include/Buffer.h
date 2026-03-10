#pragma once
#include <string>
#include "Macros.h"

class Buffer {
 public:
  Buffer() = default;
  ~Buffer() = default;

  DISALLOW_COPY_AND_MOVE(Buffer);

  /// @brief 将数据追加到缓冲区
  void Append(const char *_str, int _size);

  ssize_t Size();

  /// @brief 返回缓冲区对应的C风格字符串指针
  const char *ToStr();

  void Clear();

  /// @brief 从标准输入（控制台）读取一行数据
  void Getline();

  void SetBuf(const char *);

 private:
  std::string buf_;
};
