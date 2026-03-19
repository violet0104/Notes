#include "Connection.h"
#include "Buffer.h"
#include "Channel.h"
#include "Socket.h"
#include <assert.h>
#include <cstring>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

Connection::Connection(int fd, EventLoop *loop) {
  socket_ = std::make_unique<Socket>();
  socket_->SetFd(fd);
  if (loop != nullptr) {
    channel_ = std::make_unique<Channel>(fd, loop);
    channel_->EnableRead();
    channel_->EnableET();
  }
  read_buf_ = std::make_unique<Buffer>();
  send_buf_ = std::make_unique<Buffer>();
  state_ = State::Connected;
}

Connection::~Connection() {}

RC Connection::Read() {
  if (state_ != State::Connected) {
    perror("Connection is not connected, can not read");
    return RC_CONNECTION_ERROR;
  }
  assert(state_ == State::Connected && "connection state is disconnected!");
  read_buf_->Clear();
  if (socket_->IsNonBlocking()) {
    return ReadNonBlocking();
  } else {
    return ReadBlocking();
  }
}

RC Connection::Write() {
  if (state_ != State::Connected) {
    perror("Connection is not connected, can not write");
    return RC_CONNECTION_ERROR;
  }
  assert(state_ == State::Connected && "connection state is disconnected!");
  RC rc = RC_UNDEFINED;
  if (socket_->IsNonBlocking()) {
    rc = WriteNonBlocking();
  } else {
    rc = WriteBlocking();
  }
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
      printf("continue reading");
      continue;
    } else if (read_bytes == -1 &&
               ((errno == EAGAIN) || errno == EWOULDBLOCK)) {
      // 非阻塞IO，这个条件表示数据全部读取完毕
      break;
    } else if (read_bytes == 0) {
      // EOF，客户端断开连接
      printf("EOF, client fd %d disconnected\n", sockfd);
      state_ = State::Closed;
      Close();
      break;
    } else {
      printf("Other error on client fd %d\n", sockfd);
      state_ = State::Closed;
      Close();
      break;
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
    printf("read EOF, blocking client fd %d disconnected\n", sockfd);
    state_ = State::Closed;
  } else if (read_bytes == -1) {
    printf("Other error on blocking client fd %d\n", sockfd);
    state_ = State::Closed;
  }
  return RC_SUCCESS;
}

/// @brief Never used by server, only for client
RC Connection::WriteBlocking() {
  // 没有处理send_buffer_数据大于TCP写缓冲区，的情况，可能会有bug
  int sockfd = socket_->GetFd();
  ssize_t write_bytes = write(sockfd, send_buf_->ToStr(), send_buf_->Size());
  if (write_bytes == -1) {
    printf("Other error on blocking client fd %d\n", sockfd);
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
  Read();
  on_recv_callback_(this);
}

void Connection::SetDeleteConnectionCallback(
    std::function<void(int)> const &fn) {
  delete_connection_callback_ = std::move(fn);
}

void Connection::SetOnRecvCallback(
    std::function<void(Connection *)> const &fn) {
  on_recv_callback_ = std::move(fn);
  std::function<void()> bus = std::bind(&Connection::Business, this);
  channel_->SetReadCallback(bus);
}

void Connection::Close() { delete_connection_callback_(socket_->GetFd()); }

Connection::State Connection::GetState() { return state_; }
void Connection::SetSendBuffer(const char *str) { send_buf_->SetBuf(str); }
Buffer *Connection::GetReadBuffer() { return read_buf_.get(); }
Buffer *Connection::GetSendBuffer() { return send_buf_.get(); }
Socket *Connection::GetSocket() { return socket_.get(); }