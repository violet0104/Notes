#include "Http.h"
#include <iostream>
#include <string>

void PrintRequest(const HttpRequest &request) {
  std::cout << "Method: " << request.GetMethodString() << "\n";
  std::cout << "Path: " << request.GetPath() << "\n";
  std::cout << "Version: " << request.GetVersionString() << "\n";

  std::cout << "Headers:\n";
  for (const auto &it : request.GetHeaders()) {
    std::cout << "  " << it.first << ": " << it.second << "\n";
  }

  std::cout << "GET Params:\n";
  for (const auto &it : request.GetGetParams()) {
    std::cout << "  " << it.first << " = " << it.second << "\n";
  }

  std::cout << "POST Params:\n";
  for (const auto &it : request.GetPostParams()) {
    std::cout << "  " << it.first << " = " << it.second << "\n";
  }
  std::cout << "\n============================================\n\n";
}

int main() {
  {
    std::cout << "--- Test 1: GET Request ---\n";
    HttpParser parser;
    std::string str = "GET /hello?name=pine&age=1 HTTP/1.1\r\n"
                      "Host: 127.0.0.1:1234\r\n"
                      "Connection: keep-alive\r\n"
                      "\r\n";
    parser.Parse(str.c_str(), str.size());
    PrintRequest(parser.GetRequest());
  }

  {
    std::cout << "--- Test 2: POST Request ---\n";
    HttpParser parser;
    std::string body = "user=pine&pass=123%21"; // %21 is !
    std::string str = "POST /login HTTP/1.1\r\n"
                      "Host: 127.0.0.1:1234\r\n"
                      "Content-Length: " +
                      std::to_string(body.size()) +
                      "\r\n"
                      "Content-Type: application/x-www-form-urlencoded\r\n"
                      "\r\n" +
                      body;

    parser.Parse(str.c_str(), str.size());
    PrintRequest(parser.GetRequest());
  }

  return 0;
}
