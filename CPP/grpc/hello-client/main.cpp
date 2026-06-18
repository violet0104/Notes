#include <iostream>
#include <memory>
#include <string>

#include <grpcpp/grpcpp.h>

#include "hello.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

using helloworld::Greeter;
using helloworld::HelloReply;
using helloworld::HelloRequest;

int main(int argc, char** argv)
{
    // ============================================================
    // 1. 创建到服务器的通道（不加密）
    // ============================================================
    std::string target_address("localhost:50051");
    auto channel = grpc::CreateChannel(
        target_address,
        grpc::InsecureChannelCredentials()
    );

    // ============================================================
    // 2. 创建 Stub（桩）—— 客户端的代理对象
    // ============================================================
    std::unique_ptr<Greeter::Stub> stub = Greeter::NewStub(channel);

    // ============================================================
    // 3. 构造请求 & 发起 RPC 调用
    // ============================================================
    HelloRequest request;
    request.set_name("World");

    HelloReply reply;
    ClientContext context;

    // 同步调用 SayHello
    Status status = stub->SayHello(&context, request, &reply);

    // ============================================================
    // 4. 处理响应
    // ============================================================
    if (status.ok()) {
        std::cout << "[Client] Greeter replied: "
                  << reply.message() << std::endl;
    } else {
        std::cout << "[Client] RPC failed: "
                  << status.error_code() << ": "
                  << status.error_message() << std::endl;
    }

    return 0;
}
