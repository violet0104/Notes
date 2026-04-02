#include "Http.h"
#include "Buffer.h"
#include "Connection.h"
#include "Logger.h"
#include "Macros.h"
#include "TcpServer.h"
#include <cstddef>
#include <cstdio>
#include <functional>
#include <memory>
#include <sstream>
#include <string>

// ===================== HttpRequest 实现 =====================

HttpMethod HttpRequest::GetMethod() const { return method_; }

std::string HttpRequest::GetMethodString() const {
  switch (method_) {
  case HttpMethod::Get:
    return "GET";
  case HttpMethod::Post:
    return "POST";
  case HttpMethod::Put:
    return "PUT";
  case HttpMethod::Delete:
    return "DELETE";
  case HttpMethod::Head:
    return "HEAD";
  default:
    return "UNKNOWN";
  }
}

HttpVersion HttpRequest::GetVersion() const { return version_; }

std::string HttpRequest::GetVersionString() const {
  switch (version_) {
  case HttpVersion::Http11:
    return "HTTP/1.1";
  case HttpVersion::Http10:
    return "HTTP/1.0";
  case HttpVersion::Http20:
    return "HTTP/2.0";
  default:
    return "UNKNOWN";
  }
}

std::string HttpRequest::GetPath() const { return path_; }

std::string HttpRequest::GetHeader(const std::string &key) const {
  auto it = headers_.find(key);
  if (it != headers_.end()) {
    return it->second;
  }
  return "";
}

const std::unordered_map<std::string, std::string> &
HttpRequest::GetHeaders() const {
  return headers_;
}

std::string HttpRequest::GetBody() const { return body_; }

bool HttpRequest::IsKeepAlive() const {
  std::string connection = GetHeader("Connection");
  if (connection.empty()) {
    connection = GetHeader("connection");
  }
  if (connection == "close" || connection == "Close" || connection == "CLOSE") {
    return false;
  }
  if (connection == "keep-alive" || connection == "Keep-Alive" ||
      connection == "KEEP-ALIVE") {
    return true;
  }
  return version_ == HttpVersion::Http11;
}

std::string HttpRequest::GetGetParam(const std::string &key) const {
  auto it = get_params_.find(key);
  if (it != get_params_.end())
    return it->second;
  return "";
}

const std::unordered_map<std::string, std::string> &
HttpRequest::GetGetParams() const {
  return get_params_;
}

std::string HttpRequest::GetPostParam(const std::string &key) const {
  auto it = post_params_.find(key);
  if (it != post_params_.end())
    return it->second;
  return "";
}

const std::unordered_map<std::string, std::string> &
HttpRequest::GetPostParams() const {
  return post_params_;
}

void HttpRequest::SetMethod(HttpMethod method) { method_ = method; }

bool HttpRequest::SetMethod(const std::string &method_str) {
  if (method_str == "GET") {
    method_ = HttpMethod::Get;
  } else if (method_str == "POST") {
    method_ = HttpMethod::Post;
  } else if (method_str == "PUT") {
    method_ = HttpMethod::Put;
  } else if (method_str == "DELETE") {
    method_ = HttpMethod::Delete;
  } else if (method_str == "HEAD") {
    method_ = HttpMethod::Head;
  } else {
    method_ = HttpMethod::Unknown;
    return false;
  }
  return true;
}

void HttpRequest::SetVersion(HttpVersion version) { version_ = version; }

bool HttpRequest::SetVersion(const std::string &version_str) {
  if (version_str == "HTTP/1.1") {
    version_ = HttpVersion::Http11;
  } else if (version_str == "HTTP/1.0") {
    version_ = HttpVersion::Http10;
  } else if (version_str == "HTTP/2.0") {
    version_ = HttpVersion::Http20;
  } else {
    version_ = HttpVersion::Unknown;
    return false;
  }
  return true;
}

void HttpRequest::SetPath(const std::string &path) { path_ = path; }

void HttpRequest::SetHeader(const std::string &key, const std::string &value) {
  headers_[key] = value;
}

void HttpRequest::SetBody(const std::string &body) { body_ = body; }

void HttpRequest::SetGetParam(const std::string &key,
                              const std::string &value) {
  get_params_[key] = value;
}

void HttpRequest::SetPostParam(const std::string &key,
                               const std::string &value) {
  post_params_[key] = value;
}

void HttpRequest::Clear() {
  method_ = HttpMethod::Unknown;
  version_ = HttpVersion::Unknown;
  path_.clear();
  headers_.clear();
  body_.clear();
  get_params_.clear();
  post_params_.clear();
}

// ===================== HttpParser 实现 =====================

HttpParseState HttpParser::GetState() const { return state_; }

const HttpRequest &HttpParser::GetRequest() const { return request_; }

void HttpParser::Reset() {
  state_ = HttpParseState::RequestLine;
  request_.Clear();
}

bool HttpParser::isFinished() const { return state_ == HttpParseState::Finish; }

bool HttpParser::isError() const { return state_ == HttpParseState::Invalid; }

bool HttpParser::Parse(const std::string &str, int size) {
  Buffer temp_buf;
  temp_buf.Append(str.c_str(), size);
  return Parse(temp_buf);
}

bool HttpParser::Parse(Buffer &buffer) {
  // 状态机的主循环：只要缓冲区还有数据且状态机还在运转就一直解析
  while (buffer.HasData() && state_ != HttpParseState::Finish &&
         state_ != HttpParseState::Invalid) {
    switch (state_) {
    case HttpParseState::RequestLine: {
      // 从 Buffer 中尝试提取一行（以 \r\n 为分隔符）
      std::pair<bool, std::string> line_res = buffer.FindAndPopUntil("\r\n");
      if (!line_res.first) {
        // 没找到分隔符，说明数据不完整，暂时不处理，等待更多数据
        return false;
      }
      if (!ParseRequestLine(line_res.second)) {
        state_ = HttpParseState::Invalid;
        return false;
      }
      state_ = HttpParseState::Headers;
      break;
    }

    case HttpParseState::Headers: {
      std::pair<bool, std::string> line_res = buffer.FindAndPopUntil("\r\n");
      if (!line_res.first) {
        // 一行 Header 没收全（只有半包），等下一次数据到达
        return false;
      }
      if (line_res.second.empty()) {
        // 收到空行（连续两个 \r\n），说明 Header 结束，进入 Body 或 Finish 阶段
        std::string content_length = request_.GetHeader("Content-Length");
        if (!content_length.empty() && content_length != "0") {
          state_ = HttpParseState::Body;
        } else {
          state_ = HttpParseState::Finish;
        }
      } else {
        if (!ParseHeader(line_res.second)) {
          state_ = HttpParseState::Invalid;
          return false;
        }
      }
      break;
    }

    case HttpParseState::Body: {
      // 根据 Content-Length 提取 Body
      std::string content_length_str = request_.GetHeader("Content-Length");
      if (content_length_str.empty()) {
        state_ = HttpParseState::Finish;
        break;
      }

      size_t content_length = std::stoul(content_length_str);

      if (static_cast<size_t>(buffer.Size()) >= content_length) {
        // Body 数据已经全部到达
        std::string body(buffer.ToStr(), content_length);
        request_.SetBody(body);

        // 如果是 POST 且类型是 urlencoded，则提取参数
        if (request_.GetMethod() == HttpMethod::Post &&
            request_.GetHeader("Content-Type") ==
                "application/x-www-form-urlencoded") {
          std::unordered_map<std::string, std::string> post_params;
          ParseUrlEncoded(body, post_params);
          for (auto &pair : post_params) {
            request_.SetPostParam(pair.first, pair.second);
          }
        }

        buffer.Retrieve(content_length);
        state_ = HttpParseState::Finish;
      } else {
        // Body 数据还没有收齐（半包），等下一次读取
        return false;
      }
      break;
    }

    case HttpParseState::Finish: {
      break;
    }

    case HttpParseState::Invalid: {
      break;
    }
    }
  }
  return state_ == HttpParseState::Finish || state_ == HttpParseState::Invalid;
}

/// @brief 解析请求行，如 "GET /index.html HTTP/1.1"
/// 用空格将请求行切分为 Method, Path, Version 三段
bool HttpParser::ParseRequestLine(const std::string &line) {
  std::istringstream iss(line);
  std::string method_str;
  std::string path;
  std::string version;

  if (!(iss >> method_str >> path >> version)) {
    LOG_ERROR("HttpParser: 解析请求行失败: %s", line.c_str());
    return false;
  }

  if (!request_.SetMethod(method_str)) {
    LOG_ERROR("HttpParser: 未知的 HTTP 方法: %s", method_str.c_str());
    return false;
  }

  size_t question_mark = path.find('?');
  if (question_mark != std::string::npos) {
    std::string real_path = path.substr(0, question_mark);
    std::string query_string = path.substr(question_mark + 1);
    request_.SetPath(real_path);

    std::unordered_map<std::string, std::string> get_params;
    ParseUrlEncoded(query_string, get_params);
    for (auto &pair : get_params) {
      request_.SetGetParam(pair.first, pair.second);
    }
  } else {
    request_.SetPath(path);
  }

  if (!request_.SetVersion(version)) {
    LOG_ERROR("HttpParser: 未知的 HTTP 版本: %s", version.c_str());
    return false;
  }
  return true;
}

/// @brief 解析单行 Header，如 "Host: localhost"
/// 以第一个 ": " 为界分割为 Key 和 Value
bool HttpParser::ParseHeader(const std::string &line) {
  size_t pos = line.find(": ");
  if (pos == std::string::npos) {
    // 也尝试仅以 ":" 分割（某些客户端可能不带空格）
    pos = line.find(':');
    if (pos == std::string::npos) {
      LOG_ERROR("HttpParser: 解析 Header 失败: %s", line.c_str());
      return false;
    }
    std::string key = line.substr(0, pos);
    std::string value = line.substr(pos + 1);
    // 去除 value 前后的空格
    size_t start = value.find_first_not_of(' ');
    if (start != std::string::npos) {
      value = value.substr(start);
    }
    request_.SetHeader(key, value);
    return true;
  }

  std::string key = line.substr(0, pos);
  std::string value = line.substr(pos + 2);
  request_.SetHeader(key, value);
  return true;
}

std::string HttpParser::UrlDecode(const std::string &str) {
  std::string result;
  for (size_t i = 0; i < str.size(); ++i) {
    if (str[i] == '%') {
      if (i + 2 < str.size()) {
        int hex;
        sscanf(str.substr(i + 1, 2).c_str(), "%x", &hex);
        result += static_cast<char>(hex);
        i += 2;
      } else {
        result += '%';
      }
    } else if (str[i] == '+') {
      result += ' ';
    } else {
      result += str[i];
    }
  }
  return result;
}

/// @brief 解析 URL 编码的字符串，将类似 a=1&b=2 这样的字符串，拆解成 {"a":"1",
/// "b":"2"}
void HttpParser::ParseUrlEncoded(
    const std::string &str,
    std::unordered_map<std::string, std::string> &params) {
  if (str.empty()) {
    return;
  }
  std::string::size_type start = 0;
  std::string::size_type end = str.find('&');
  while (end != std::string::npos) {
    std::string pair = str.substr(start, end - start);
    std::string::size_type eq = pair.find('=');
    if (eq != std::string::npos) {
      params[UrlDecode(pair.substr(0, eq))] = UrlDecode(pair.substr(eq + 1));
    } else {
      params[UrlDecode(pair)] = "";
    }
    start = end + 1;
    end = str.find('&', start);
  }
  std::string pair = str.substr(start);
  if (!pair.empty()) {
    std::string::size_type eq = pair.find('=');
    if (eq != std::string::npos) {
      params[UrlDecode(pair.substr(0, eq))] = UrlDecode(pair.substr(eq + 1));
    } else {
      params[UrlDecode(pair)] = "";
    }
  }
}

// ===================== HttpResponse 实现 =====================

void HttpResponse::SetStatusCode(HttpStatusCode code) { status_code_ = code; }

void HttpResponse::SetHeader(const std::string &key, const std::string &value) {
  headers_[key] = value;
}

void HttpResponse::SetBody(const std::string &body) { body_ = body; }

std::string HttpResponse::GetStatusCodeString(HttpStatusCode code) {
  switch (code) {
  case HttpStatusCode::Ok:
    return "200 OK";
  case HttpStatusCode::BadRequest:
    return "400 Bad Request";
  case HttpStatusCode::NotFound:
    return "404 Not Found";
  case HttpStatusCode::InternalServerError:
    return "500 Internal Server Error";
  default:
    return "Unknown";
  }
}

std::string HttpResponse::SerializeToString() const {
  std::string response;
  // HTTP/1.1 200 OK
  response += "HTTP/1.1 " + GetStatusCodeString(status_code_) + "\r\n";

  // Content-Length Auto-fill
  std::string body_length = std::to_string(body_.size());
  bool has_content_length = false;

  for (const auto &header : headers_) {
    response += header.first + ": " + header.second + "\r\n";
    if (header.first == "Content-Length") {
      has_content_length = true;
    }
  }

  if (!has_content_length) {
    response += "Content-Length: " + body_length + "\r\n";
  }

  response += "\r\n";
  response += body_;

  return response;
}

// ===================== HttpServer 实现 =====================
HttpServer::HttpServer(const char *ip, uint16_t port) {
  http_server_ = std::unique_ptr<TcpServer>(new TcpServer(ip, port));
}

HttpServer::~HttpServer() {}

void HttpServer::Start() {
  http_server_->OnRecv([this](std::shared_ptr<Connection> conn) {
    HttpParser *parser = conn->GetHttpParser();
    parser->Parse(*conn->GetReadBuffer());

    if (parser->isFinished()) {
      const HttpRequest &req = parser->GetRequest();
      HttpResponse res;
      bool is_keep_alive = req.IsKeepAlive();
      if (is_keep_alive) {
        res.SetHeader("Connection", "keep-alive");
      } else {
        res.SetHeader("Connection", "close");
      }

      if (on_request_callback_) {
        on_request_callback_(req, res);
      }
      conn->Send(res.SerializeToString());

      if (is_keep_alive) {
        // 单次 HTTP 请求处理完毕。重置上下文以支持后续可能的 Keep-Alive 复用该
        // TCP 请求
        parser->Reset();
      } else {
        conn->Close();
      }
    }
  });
  LOG_INFO("Http Server starts running! Please visit http://127.0.0.1:1234/ in "
           "your browser.");
  http_server_->Start();
}

void HttpServer::Stop() { http_server_->Stop(); }

void HttpServer::SetOnRequestCallback(
    std::function<void(const HttpRequest &, HttpResponse &)> fn) {
  on_request_callback_ = std::move(fn);
}