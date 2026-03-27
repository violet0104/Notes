#include "Connection.h"
#include "Buffer.h"
#include "Channel.h"
#include "Logger.h"
#include "Socket.h"
#include <assert.h>
#include <cstring>
#include <string.h>
#include <unistd.h>

Connection::Connection(int fd, EventLoop *loop) {
  socket_ = std::unique_ptr<Socket>(new Socket());
  socket_->SetFd(fd);
  if (loop != nullptr) {
    channel_ = std::unique_ptr<Channel>(new Channel(fd, loop));
    channel_->EnableRead();
    channel_->EnableET();
  }
  read_buf_ = std::unique_ptr<Buffer>(new Buffer());
  send_buf_ = std::unique_ptr<Buffer>(new Buffer());
  state_ = State::Connected;
  http_parser_ = std::unique_ptr<HttpParser>(new HttpParser());
  UpdateLastActiveTime();
}

Connection::~Connection() {}

RC Connection::Read() {
  if (state_ != State::Connected) {
    LOG_ERROR("Connection is not connected, can not read");
    return RC_CONNECTION_ERROR;
  }
  assert(state_ == State::Connected && "connection state is disconnected!");
  read_buf_->Clear();
  UpdateLastActiveTime();
  if (socket_->IsNonBlocking()) {
    return ReadNonBlocking();
  } else {
    return ReadBlocking();
  }
}

RC Connection::Write() {
  if (state_ != State::Connected) {
    LOG_ERROR("Connection is not connected, can not write");
    return RC_CONNECTION_ERROR;
  }
  assert(state_ == State::Connected && "connection state is disconnected!");
  RC rc = RC_UNDEFINED;
  if (socket_->IsNonBlocking()) {
    rc = WriteNonBlocking();
  } else {
    rc = WriteBlocking();
  }
  UpdateLastActiveTime();
  send_buf_->Clear();
  return rc;
}

RC Connection::ReadNonBlocking() {
  int sockfd = socket_->GetFd();
  char buf[1024]; // 这个buf大小无所谓
  // 使用非阻塞IO，读取客户端buffer，一次读取buf大小数据，直到全部读取完毕
  while (true) {
    std::memset(buf, 0, sizeof(buf));
    ssize_t read_bytes = read(sockfd, buf, sizeof(buf));
    if (read_bytes > 0) {
      read_buf_->Append(buf, read_bytes);
    } else if (read_bytes == -1 && errno == EINTR) {
      // 客户端正常中断、继续读取
      LOG_DEBUG("continue reading");
      continue;
    } else if (read_bytes == -1 &&
               ((errno == EAGAIN) || errno == EWOULDBLOCK)) {
      // 非阻塞IO，这个条件表示数据全部读取完毕
      break;
    } else if (read_bytes == 0) {
      // EOF，客户端断开连接
      LOG_INFO("EOF, client fd %d disconnected", sockfd);
      state_ = State::Closed;
      Close();
      return RC_CONNECTION_ERROR;
    } else {
      LOG_ERROR("Other error on client fd %d: %s", sockfd, strerror(errno));
      state_ = State::Closed;
      Close();
      return RC_CONNECTION_ERROR;
    }
  }
  return RC_SUCCESS;
}

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
      LOG_DEBUG("continue writing");
      continue;
    }
    if (write_bytes == -1 && errno == EAGAIN) {
      break;
    }
    if (write_bytes == -1) {
      LOG_ERROR("Other error on client fd %d: %s", sockfd, strerror(errno));
      state_ = State::Closed;
      break;
    }
    send_buf_->Retrieve(write_bytes);
  }
  return RC_SUCCESS;
}

/// @brief Never used by server, only for client
RC Connection::ReadBlocking() {
  int sockfd = socket_->GetFd();
  // unsigned int rcv_size = 0;
  // socklen_t len = sizeof(rcv_size);
  // size_t data_size = socket_->RecvBufSize();
  char buf[1024];
  ssize_t read_bytes = read(sockfd, buf, sizeof(buf));
  if (read_bytes > 0) {
    read_buf_->Append(buf, read_bytes);
  } else if (read_bytes == 0) {
    LOG_INFO("read EOF, blocking client fd %d disconnected", sockfd);
    state_ = State::Closed;
    return RC_CONNECTION_ERROR;
  } else if (read_bytes == -1) {
    LOG_ERROR("Other error on blocking client fd %d: %s", sockfd,
              strerror(errno));
    state_ = State::Closed;
    return RC_CONNECTION_ERROR;
  }
  return RC_SUCCESS;
}

/// @brief Never used by server, only for client
RC Connection::WriteBlocking() {
  // 没有处理send_buffer_数据大于TCP写缓冲区，的情况，可能会有bug
  int sockfd = socket_->GetFd();
  ssize_t write_bytes = write(sockfd, send_buf_->ToStr(), send_buf_->Size());
  if (write_bytes == -1) {
    LOG_ERROR("Other error on blocking client fd %d: %s", sockfd,
              strerror(errno));
    state_ = State::Closed;
    Close();
  }
  return RC_SUCCESS;
}

RC Connection::Send(std::string msg) {
  SetSendBuffer(msg.c_str());
  Write();
  return RC_SUCCESS;
}

void Connection::Business() {
  std::shared_ptr<Connection> guard = shared_from_this();
  UpdateLastActiveTime();
  RC rc = Read();
  if (rc == RC_SUCCESS && on_recv_callback_) {
    on_recv_callback_(guard);
  }
}

void Connection::SetDeleteConnectionCallback(
    std::function<void(int)> const &fn) {
  delete_connection_callback_ = std::move(fn);
}

void Connection::SetOnRecvCallback(std::function<void(std::shared_ptr<Connection>)> const &fn) {
  on_recv_callback_ = std::move(fn);
  std::weak_ptr<Connection> weak_self = shared_from_this();
  auto bus = [weak_self]() {
    if (std::shared_ptr<Connection> self = weak_self.lock()) {
      self->Business();
    }
  };
  channel_->SetReadCallback(bus);
}

void Connection::Close() {
  if (state_ == State::Closed) {
    return;
  }
  state_ = State::Closed;
  if (channel_) {
    channel_->DisableAll();
    channel_->SetReadCallback(nullptr);
    channel_->SetWriteCallback(nullptr);
    // channel_->Remove(); // 如果 Channel 类有这个方法则调用
  }
  delete_connection_callback_(socket_->GetFd());
}

Connection::State Connection::GetState() { return state_; }
void Connection::SetSendBuffer(const char *str) { send_buf_->SetBuf(str); }
Buffer *Connection::GetReadBuffer() { return read_buf_.get(); }
Buffer *Connection::GetSendBuffer() { return send_buf_.get(); }
Socket *Connection::GetSocket() { return socket_.get(); }

void Connection::SetHttpParser(std::unique_ptr<HttpParser> parser) {
  http_parser_ = std::move(parser);
}
HttpParser *Connection::GetHttpParser() { return http_parser_.get(); }
void Connection::ClearHttpParser() { http_parser_.reset(); }

void Connection::UpdateLastActiveTime() {
  last_active_time_ = std::chrono::steady_clock::now();
}
std::chrono::steady_clock::time_point Connection::GetLastActiveTime() const {
  return last_active_time_;
}