#include "Http.h"
#include "pine.h"
#include <fstream>
#include <iostream>
#include <sstream>

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
  std::cout << "接收到 HTTP 请求: " << req.GetMethodString() << " " << path
            << std::endl;

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
    std::string content = ReadFile("../test/www/index.html");
    if (!content.empty()) {
      res.SetStatusCode(HttpStatusCode::Ok);
      res.SetBody(content);
      res.SetHeader("Content-Type", "text/html; charset=utf-8");
    } else {
      res.SetStatusCode(HttpStatusCode::NotFound);
      res.SetBody("404 Not Found - Missing index.html");
    }
  } else {
    std::string content = ReadFile("../test/www/404.html");
    res.SetStatusCode(HttpStatusCode::NotFound);
    res.SetHeader("Content-Type", "text/html; charset=utf-8");
    if (!content.empty()) {
      res.SetBody(content);
    } else {
      res.SetBody("404 Not Found");
    }
  }
}

int main() {
  HttpServer *server = new HttpServer("127.0.0.1", 1234);

  Signal::signal(SIGINT, [&] {
    std::cout << "\nServer exit!" << std::endl;
    server->Stop();
  });

  server->SetOnRequestCallback(OnRequest);
  server->Start();

  delete server;
  return 0;
}
