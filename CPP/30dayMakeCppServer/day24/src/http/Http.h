#pragma once
#include "Macros.h"
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

/// @brief HTTP 请求方法
enum class HttpMethod { Get, Post, Put, Delete, Head, Unknown };

/// @brief HTTP 版本
enum class HttpVersion { Http11, Http10, Http20, Unknown };

/// @brief HTTP 状态码
enum class HttpStatusCode {
  Ok = 200,
  BadRequest = 400,
  NotFound = 404,
  InternalServerError = 500,
  Unknown
};

/// @brief HTTP 解析状态
enum class HttpParseState {
  RequestLine, // 解析请求行
  Headers,     // 解析请求头
  Body,        // 解析请求体
  Finish,      // 解析完成
  Invalid,     // 解析出错
};

/// @brief 存储一个完整的 HTTP 请求的解析结果
class HttpRequest {
public:
  HttpRequest() = default;
  ~HttpRequest() = default;
  DISALLOW_COPY_AND_MOVE(HttpRequest);

  HttpMethod GetMethod() const;
  std::string GetMethodString() const;
  HttpVersion GetVersion() const;
  std::string GetVersionString() const;
  std::string GetPath() const;
  std::string GetHeader(const std::string &key) const;
  const std::unordered_map<std::string, std::string> &GetHeaders() const;
  std::string GetBody() const;

  bool IsKeepAlive() const;

  std::string GetGetParam(const std::string &key) const;
  const std::unordered_map<std::string, std::string> &GetGetParams() const;
  std::string GetPostParam(const std::string &key) const;
  const std::unordered_map<std::string, std::string> &GetPostParams() const;

  void SetMethod(HttpMethod method);
  bool SetMethod(const std::string &method_str);
  void SetVersion(HttpVersion version);
  bool SetVersion(const std::string &version_str);
  void SetPath(const std::string &path);
  void SetHeader(const std::string &key, const std::string &value);
  void SetBody(const std::string &body);

  void SetGetParam(const std::string &key, const std::string &value);
  void SetPostParam(const std::string &key, const std::string &value);

  void Clear();

private:
  HttpMethod method_;
  HttpVersion version_;
  std::string path_;
  std::unordered_map<std::string, std::string> headers_;
  std::string body_;
  std::unordered_map<std::string, std::string> get_params_;
  std::unordered_map<std::string, std::string> post_params_;
};

/// @brief 基于有限状态机 (FSM) 的 HTTP 请求解析器
class HttpParser {
public:
  HttpParser() = default;
  ~HttpParser() = default;
  DISALLOW_COPY_AND_MOVE(HttpParser);

  bool Parse(const std::string &str, int size);
  /// @return true 表示解析完成（Finish 或 Invalid），false 表示还需要更多数据
  bool Parse(Buffer &buffer);
  /// @brief 解析请求行，如 "GET /index.html HTTP/1.1"
  bool ParseRequestLine(const std::string &line);
  /// @brief 解析单行 Header，如 "Host: localhost"
  bool ParseHeader(const std::string &line);

  HttpParseState GetState() const;
  const HttpRequest &GetRequest() const;
  void Reset();
  bool isFinished() const;
  bool isError() const;

private:
  HttpParseState state_{HttpParseState::RequestLine};
  HttpRequest request_;

  static std::string UrlDecode(const std::string &str);
  static void
  ParseUrlEncoded(const std::string &str,
                  std::unordered_map<std::string, std::string> &params);
};

/// @brief HTTP 响应
class HttpResponse {
public:
  HttpResponse() = default;
  ~HttpResponse() = default;
  DISALLOW_COPY_AND_MOVE(HttpResponse);

  void SetStatusCode(HttpStatusCode code);
  void SetHeader(const std::string &key, const std::string &value);
  void SetBody(const std::string &body);

  std::string SerializeToString() const;

private:
  HttpStatusCode status_code_{HttpStatusCode::Unknown};
  std::unordered_map<std::string, std::string> headers_;
  std::string body_;

  static std::string GetStatusCodeString(HttpStatusCode code);
};

/// @brief HTTP 服务器
class HttpServer {
public:
  HttpServer(const char *ip, uint16_t port);
  ~HttpServer();
  DISALLOW_COPY_AND_MOVE(HttpServer);

  void Start();
  void SetOnRequestCallback(
      std::function<void(const HttpRequest &, HttpResponse &)> fn);

private:
  std::unique_ptr<TcpServer> http_server_;
  std::function<void(const HttpRequest &, HttpResponse &)> on_request_callback_;
};