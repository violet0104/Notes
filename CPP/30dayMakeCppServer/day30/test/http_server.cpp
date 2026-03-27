#include "Http.h"
#include "pine.h"
#include <fstream>
#include <iostream>
#include <sstream>

std::string g_index_content;
std::string g_404_content;

std::string ReadFile(const std::string &filepath) {
  std::ifstream file(filepath);
  if (!file.is_open()) {
    return "";
  }
  std::stringstream buffer;
  buffer << file.rdbuf();
  return buffer.str();
}

void OnRequest(const HttpRequest &req, HttpResponse &res) {
  std::string path = req.GetPath();
  // 已移除：std::cout << "接收到 HTTP 请求: " << req.GetMethodString() << " " << path << std::endl;

  if (req.GetMethod() == HttpMethod::Post && path == "/api/login") {
    std::string username = req.GetPostParam("username");
    std::string password = req.GetPostParam("password");
    std::string body =
        "<html><head><meta charset='utf-8'></head><body style='font-family: "
        "sans-serif; text-align: center; padding: 50px;'>";
    body += "<h2>登录结果</h2>";
    if (username == "violet" && password == "123456") {
      body += "<p style='color:green; font-size: 1.2em;'>登录成功！欢迎回来，" +
              username + " 🎉</p>";
    } else {
      body += "<p style='color:red; font-size: "
              "1.2em;'>登录失败，用户名或密码错误！</p>";
    }
    body += "<br><a href='/' style='color:#007bff; "
            "text-decoration:none;'>返回首页</a></body></html>";
    res.SetStatusCode(HttpStatusCode::Ok);
    res.SetHeader("Content-Type", "text/html; charset=utf-8");
    res.SetBody(body);
  } else if (path == "/" || path == "/index.html") {
    if (!g_index_content.empty()) {
      res.SetStatusCode(HttpStatusCode::Ok);
      res.SetBody(g_index_content);
      res.SetHeader("Content-Type", "text/html; charset=utf-8");
    } else {
      res.SetStatusCode(HttpStatusCode::NotFound);
      res.SetBody("404 Not Found - Missing index.html");
    }
  } else {
    res.SetStatusCode(HttpStatusCode::NotFound);
    res.SetHeader("Content-Type", "text/html; charset=utf-8");
    if (!g_404_content.empty()) {
      res.SetBody(g_404_content);
    } else {
      res.SetBody("404 Not Found");
    }
  }
}

int main() {
  g_index_content = ReadFile("../test/www/index.html");
  g_404_content = ReadFile("../test/www/404.html");

  HttpServer *server = new HttpServer("127.0.0.1", 1234);

  auto shutdown_handler = [&] {
    std::cout << "\nServer exit!" << std::endl;
    server->Stop();
  };

  Signal::signal(SIGINT, shutdown_handler);
  Signal::signal(SIGTERM, shutdown_handler);

  server->SetOnRequestCallback(OnRequest);
  server->Start();

  delete server;
  return 0;
}
