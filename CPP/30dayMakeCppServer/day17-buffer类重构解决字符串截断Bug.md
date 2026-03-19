# day17-buffer类重构解决字符串截断 Bug

TCP 是基于字节流的协议，这意味着数据并没有固定的“消息边界”。当前的 `Buffer` 实现仅仅是简单地追加数据、一次性读取全部缓冲，这不足以应对真实网络环境中必然出现的“粘包”（多个包连在一起）和“半包”（一个包被截断分批到达）问题。我们需要重构 `Buffer` 类，提供基于协议（例如以 `\r\n` 分隔）提取完整消息的方法。

我们将重构 `Buffer` 使其内部使用 `std::string`，添加按长度或特定分隔符检索数据的方法，并更新 `Connection` 以使用这些新特性。

**Buffer.h**

- 新增 `FindAndPopUntil(const std::string &delim)`：查找分隔符（如 `\r\n`），如果找到则提取分隔符前的这段完整消息，并将其从缓冲区中弹出（Pop）。
- 新增 `HasData()`：判断缓冲区是否为空，用于循环提取。
- 新增 `Retrieve(size_t len)`：从缓冲区头部弹出指定字节长度的数据（当我们确认数据已经被消费后调用）。

```cpp
#include <optional>

class Buffer {
public:
  /// @brief 尝试寻找特定的分隔符，如果找到，则提取前面的数据并从缓冲区弹出
  /// @return 提取出的完整消息，如果没找到分隔符返回 nullopt
  std::optional<std::string> FindAndPopUntil(const std::string &delim);

  /// @brief 判断缓冲区是否有数据
  bool HasData() const;

  /// @brief 消费掉缓冲区头部的长度为len的数据
  void Retrieve(size_t len);
};
```

**Buffer.cpp**

- 使用 `buf_.find()` 和 `buf_.substr()` 实现 `FindAndPopUntil` 方法。
- 实现 `Retrieve(size_t len)` 用于丢弃已消费的字节。
- **严重 Bug 修复**：更新 `Append` 方法。目前的 `Append` 中会遇到 `\0` 就 `break` 停止读取。在实际的二级制网络流中，包含 `\0` 是非常正常的。必须改为 `buf_.append(_str, _size)` 直接追加，允许内部存在 null 字符。

```cpp
#include "Buffer.h"
#include <optional>

void Buffer::Append(const char *_str, int _size) {
  buf_.append(_str, _size); // 修复了遇 \0 截断的bug
}

std::optional<std::string> Buffer::FindAndPopUntil(const std::string &delim) {
  size_t pos = buf_.find(delim);
  if (pos == std::string::npos) {
    return std::nullopt;
  }
  std::string res = buf_.substr(0, pos);
  buf_.erase(0, pos + delim.size());
  return res;
}

bool Buffer::HasData() const {
  return !buf_.empty();
}

void Buffer::Retrieve(size_t len) {
  if (len > buf_.size()) {
    len = buf_.size();
  }
  buf_.erase(0, len);
}
```

**Connection.cpp**

- 修改 `WriteNonBlocking`：目前它是假设整个 `send_buf_` 被一次性发完的。如果发生局部写入（Partial Write），必须根据系统调用 `write()` 返回的真实写入字节数，调用 `send_buf_->Retrieve(write_bytes)` 来剔除已经成功发出去的数据，留下还没发出去的继续重试。
- 目前 `Connection::ReadNonBlocking` 已经把接收到的数据无脑 `Append` 进了 `read_buf_`，这是完全正确的。将来应用层（如 HTTP 解析器）只需要拿着 `read_buf_` 持续调用 `FindAndPopUntil()` 即可完美解决粘包。

```cpp
RC Connection::WriteNonBlocking() {
  int sockfd = socket_->GetFd();
  char buf[1024];
  while (send_buf_->HasData()) {
    int data_size = send_buf_->Size();
    if (data_size > (int)sizeof(buf)) {
      data_size = sizeof(buf);
    }
    memcpy(buf, send_buf_->ToStr(), data_size);
    ssize_t write_bytes = write(sockfd, buf, data_size);
    if (write_bytes == -1 && errno == EINTR) {
      printf("continue writing\n");
      continue;
    }
    if (write_bytes == -1 && errno == EAGAIN) {
      break;
    }
    if (write_bytes == -1) {
      printf("Other error on client fd %d\n", sockfd);
      state_ = State::Closed;
      break;
    }
    send_buf_->Retrieve(write_bytes); // 根据真实发出的字节数清理 buffer
  }
  return RC_SUCCESS;
}
```



**buffer_test.cpp单元测试：**

新建一个单元测试，专门验证 `Buffer` 类能否正确地处理连包、半包，并且验证包含 `\0` 的二进制数据是否还会被错误截断。

---

以上即为 Day 17 重构 `Buffer` 处理数据边界的所有核心步骤。现在这部分所有的改动都已经通过编译并利用 `buffer_test` 测试通过，解决了长期潜伏的字符串截断 Bug，使得网络层能够真正平稳地应对复杂的 TCP 数据流解析工作。
