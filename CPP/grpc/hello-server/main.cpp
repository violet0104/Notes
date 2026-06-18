#include <iostream>
#include <memory>
#include <string>

#include <grpcpp/grpcpp.h>

#include "hello.grpc.pb.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

using helloworld::Greeter;
using helloworld::HelloReply;
using helloworld::HelloRequest;

// ============================================================
// 1. 继承生成的服务基类，实现 RPC 方法
// ============================================================
class GreeterServiceImpl final : public Greeter::Service {
public:
    // SayHello 的签名由 .proto 决定：
    // rpc SayHello(HelloRequest) returns (HelloReply);
    Status SayHello(ServerContext* context, const HelloRequest* request,
                    HelloReply* reply) override
    {
        // 从 request 中取出客户端发来的 name
        std::string prefix("Hello, ");
        reply->set_message(prefix + request->name());

        std::cout << "[Server] Received request from: "
                  << request->name() << std::endl;

        return Status::OK;
    }
};

// ============================================================
// 2. 启动 Server
// ============================================================
int main(int argc, char** argv)
{
    std::string server_address("0.0.0.0:50051");
    GreeterServiceImpl service;

    ServerBuilder builder;
    // 监听地址 + 不加密的凭证（生产环境请使用 SSL）
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    // 注册服务
    builder.RegisterService(&service);

    // 构建并启动
    std::unique_ptr<Server> server(builder.BuildAndStart());
    std::cout << "[Server] Listening on " << server_address << std::endl;

    // 阻塞直到服务器被关闭（Ctrl+C）
    server->Wait();
    return 0;
}
