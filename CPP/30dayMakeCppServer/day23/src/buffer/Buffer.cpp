#include "Buffer.h"
#include <iostream>
#include <string>

void Buffer::Append(const char *_str, int _size) { buf_.append(_str, _size); }

ssize_t Buffer::Size() { return buf_.size(); }

const char *Buffer::ToStr() { return buf_.c_str(); }

void Buffer::Clear() { buf_.clear(); }

void Buffer::Getline() {
  buf_.clear();
  std::getline(std::cin, buf_);
}

void Buffer::SetBuf(const char *buf) {
  buf_.clear();
  buf_.append(buf);
}

std::pair<bool, std::string> Buffer::FindAndPopUntil(const std::string &delim) {
  size_t pos = buf_.find(delim);
  if (pos == std::string::npos) {
    return std::make_pair(false, "");
  }
  std::string result = buf_.substr(0, pos);
  buf_.erase(0, pos + delim.size());
  return std::make_pair(true, result);
}

bool Buffer::HasData() const { return !buf_.empty(); }

void Buffer::Retrieve(size_t len) {
  if (len > buf_.size()) {
    len = buf_.size();
  }
  buf_.erase(0, len);
}
