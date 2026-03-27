#pragma once
#include "Macros.h"
#include <string>
#include <utility>

class Buffer {
public:
  Buffer() = default;
  ~Buffer() = default;

  DISALLOW_COPY_AND_MOVE(Buffer);

  void Append(const char *_str, int _size);

  ssize_t Size();

  /// @brief 返回缓冲区对应的C风格字符串指针
  const char *ToStr();

  void Clear();

  /// @brief 从标准输入（控制台）读取一行数据
  void Getline();

  void SetBuf(const char *);

  /// @return <是否找到, 提取出的完整消息>
  std::pair<bool, std::string> FindAndPopUntil(const std::string &delim);

  /// @brief 判断缓冲区是否有数据
  bool HasData() const;

  /// @brief 消费掉缓冲区头部的长度为len的数据
  void Retrieve(size_t len);

private:
  std::string buf_;
};
