#pragma once
#include "common.h"
#include <optional>
#include <string>

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

  /// @brief 尝试寻找特定的分隔符，如果找到，则提取前面的数据并从缓冲区弹出
  /// @return 提取出的完整消息，如果没找到分隔符返回 nullopt
  std::optional<std::string> FindAndPopUntil(const std::string &delim);

  /// @brief 判断缓冲区是否有数据
  bool HasData() const;

  /// @brief 消费掉缓冲区头部的长度为len的数据
  void Retrieve(size_t len);

private:
  std::string buf_;
};
