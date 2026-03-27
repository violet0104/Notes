#include "AsyncLogging.h"
#include "LogFile.h"
#include <chrono>
#include <cassert>
#include <cstdio>
#include <cstring>

AsyncLogging::AsyncLogging(const std::string &basename, off_t rollSize,
                           int flushInterval)
    : flushInterval_(flushInterval), running_(false), basename_(basename),
      rollSize_(rollSize), thread_(), mutex_(), cond_(),
      currentBuffer_(new Buffer), nextBuffer_(new Buffer), buffers_() {
  currentBuffer_->bzero();
  nextBuffer_->bzero();
  buffers_.reserve(16);
  running_ = true;
  thread_ = std::thread(&AsyncLogging::ThreadFunc, this);
}

AsyncLogging::~AsyncLogging() {
  if (running_) {
    Stop();
  }
  delete currentBuffer_;
  delete nextBuffer_;
  for (auto buffer : buffers_) {
    delete buffer;
  }
}

void AsyncLogging::Append(const char *logline, int len) {
  std::lock_guard<std::mutex> lock(mutex_);
  if (currentBuffer_->avail() > len) {
    currentBuffer_->append(logline, len);
  } else {
    buffers_.push_back(currentBuffer_);
    if (nextBuffer_) {
      currentBuffer_ = nextBuffer_;
      nextBuffer_ = nullptr;
    } else {
      // 备用缓冲区也用完了（很少见，高并发写太快），只能开辟新缓冲区
      currentBuffer_ = new Buffer;
    }
    currentBuffer_->append(logline, len);
    cond_.notify_one();
  }
}

void AsyncLogging::Stop() {
  running_ = false;
  cond_.notify_one(); // 唤醒后台线程
  if (thread_.joinable()) {
    thread_.join(); // 等待后台线程结束
  }
}

void AsyncLogging::ThreadFunc() {
  LogFile output(basename_, rollSize_, flushInterval_);
  BufferPtr newBuffer1 = new Buffer;
  BufferPtr newBuffer2 = new Buffer;
  newBuffer1->bzero();
  newBuffer2->bzero();
  std::vector<BufferPtr> buffersToWrite;
  buffersToWrite.reserve(16);

  while (running_) {
    assert(newBuffer1 && newBuffer1->length() == 0);
    assert(newBuffer2 && newBuffer2->length() == 0);

    {
      std::unique_lock<std::mutex> lock(mutex_);
      if (buffers_.empty()) {
        cond_.wait_for(lock, std::chrono::seconds(flushInterval_));
      }
      buffers_.push_back(currentBuffer_);
      currentBuffer_ = newBuffer1;
      newBuffer1 = nullptr;
      buffersToWrite.swap(buffers_); // 交换前端和后端的缓冲区
      if (!nextBuffer_) {
        nextBuffer_ = newBuffer2;
        newBuffer2 = nullptr;
      }
    }

    if (buffersToWrite.empty()) {
      continue;
    }

    // 如果待写入数据太多（堆积超过 25 个），则丢弃冗余部分，保持服务器稳健
    if (buffersToWrite.size() > 25) {
      char buf[256];
      snprintf(buf, sizeof(buf),
               "Dropped log messages at %s, %zd larger buffers\n", "sometime",
               buffersToWrite.size() - 2);
      fputs(buf, stderr);
      output.Append(buf, static_cast<int>(strlen(buf)));
      for (size_t i = 2; i < buffersToWrite.size(); ++i) {
        delete buffersToWrite[i];
      }
      buffersToWrite.erase(buffersToWrite.begin() + 2, buffersToWrite.end());
    }

    for (const auto &buffer : buffersToWrite) {
      output.Append(buffer->data(), buffer->length());
    }

    // 在写完磁盘后清理并回收
    if (buffersToWrite.size() > 2) {
      // 必须先把 2 以后的对象 delete 掉，否则会内存泄漏！
      for (size_t i = 2; i < buffersToWrite.size(); ++i) {
        delete buffersToWrite[i];
      }
      buffersToWrite.resize(2);
    }

    if (!newBuffer1) {
      assert(!buffersToWrite.empty());
      newBuffer1 = buffersToWrite.back();
      buffersToWrite.pop_back();
      newBuffer1->reset();
    }

    if (!newBuffer2) {
      assert(!buffersToWrite.empty());
      newBuffer2 = buffersToWrite.back();
      buffersToWrite.pop_back();
      newBuffer2->reset();
    }

    buffersToWrite.clear();
    output.Flush();
  }
  output.Flush();
}
