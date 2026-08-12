# gRPC C++ 入门教程

> [gRPC 官方文档中文版](https://doc.oschina.net/grpc?t=56831)

这份教程基于当前目录下的示例项目编写，目标是帮助你快速理解：

- `gRPC` 在这个项目里是怎么组织的
- `proto` 文件如何定义接口
- `C++` 服务端和客户端分别怎么写
- `CMake` 如何自动生成和链接 gRPC 代码
- 我们应该怎么编译和运行这个示例

这个项目实现的是一个最小可运行的问候服务：客户端发送名字，服务端返回一句 `Hello, xxx`。

## 1. 项目结构

当前项目的核心目录如下：

```text
grpc/
├── CMakeLists.txt
├── hello-server/
│   ├── main.cpp
│   └── proto/
│       └── hello.proto
├── hello-client/
│   └── main.cpp
└── build/
    ├── hello-server
    ├── hello-client
    └── generated/
```

其中：

- `hello-server/proto/hello.proto`：定义消息结构和 RPC 接口
- `hello-server/main.cpp`：服务端实现
- `hello-client/main.cpp`：客户端调用
- `CMakeLists.txt`：负责查找依赖、生成代码、编译可执行文件
- `build/generated/`：由 `protoc` 和 `grpc_cpp_plugin` 生成的 C++ 代码

## 2. 什么是 gRPC

可以先把 gRPC 理解成一种“远程函数调用”框架。

平时我们在本地调用函数是这样：

```cpp
std::string result = SayHello("World");
```

而使用 gRPC 之后，客户端看起来仍然像在调用一个函数，但这个函数的真正执行位置是在另一台进程或服务器上。

gRPC 帮我们处理了这些麻烦事：

- 请求和响应的序列化
- 网络传输
- 客户端和服务端的接口对齐
- 跨语言支持

在这个示例里，远程调用的方法名叫 `SayHello`。

## 3. 从 `.proto` 文件开始

gRPC 项目最核心的入口，通常就是 `.proto` 文件。

当前项目中的文件是 `hello.proto`：

```protobuf
syntax = "proto3";

package helloworld;

service Greeter {
  rpc SayHello (HelloRequest) returns (HelloReply) {}
}

message HelloRequest {
  string name = 1;
}

message HelloReply {
  string message = 1;
}
```

这段定义可以拆成三部分理解。

### 3.1 `syntax = "proto3";`

表示使用 Protocol Buffers 第 3 版语法，也就是现代 gRPC 项目里最常见的写法。

### 3.2 `package helloworld;`

这会影响生成代码中的命名空间。当前项目里生成后的类型位于 `helloworld` 命名空间下，例如：

- `helloworld::Greeter`
- `helloworld::HelloRequest`
- `helloworld::HelloReply`

### 3.3 `service` 和 `message`

`service Greeter` 定义了一个服务，服务里有一个 RPC 方法：

```protobuf
rpc SayHello (HelloRequest) returns (HelloReply) {}
```

它的意思是：

- 客户端发送一个 `HelloRequest`
- 服务端返回一个 `HelloReply`

而 `message` 可以理解为跨网络传输的数据结构：

- `HelloRequest` 里有一个字段 `name`
- `HelloReply` 里有一个字段 `message`

```protobuf
// message关键字，可以理解为c++中的结构体。
// 这里比较特殊的是变量后面的"赋值"。注意，这里并不是赋值，而是在定义这个变量在这个message中的位置。
message HelloRequest {
  string name = 1;
//   int64 age = 2;
}

// The response message containing the greetings
message HelloReply {
  string message = 1;
}
```

字段编号 `= 1` 不是赋值，而是 Protocol Buffers 内部使用的字段标识。这个编号一旦对外发布，后续就不应该随便改动。

## 4. gRPC 代码是怎么生成的

项目里的 `CMakeLists.txt`已经把自动生成流程写好了。

关键步骤如下：

### 4.1 查找依赖

```cmake
find_package(protobuf CONFIG REQUIRED)
find_package(gRPC CONFIG REQUIRED)
```

这两句会找到：

- Protocol Buffers 编译器和库
- gRPC 的头文件、库以及代码生成插件

### 4.2 指定 `.proto` 文件位置

```cmake
set(PROTO_DIR "${CMAKE_CURRENT_SOURCE_DIR}/hello-server/proto")
set(PROTO_FILE "${PROTO_DIR}/hello.proto")
set(GEN_DIR  "${CMAKE_CURRENT_BINARY_DIR}/generated")
```

这里说明：

- 源 proto 文件在 `hello-server/proto/`
- 生成代码输出到 `build/generated/`

### 4.3 调用 `protoc` 和 `grpc_cpp_plugin`

```cmake
add_custom_command(
    OUTPUT
        ${GEN_DIR}/hello.pb.cc
        ${GEN_DIR}/hello.pb.h
        ${GEN_DIR}/hello.grpc.pb.cc
        ${GEN_DIR}/hello.grpc.pb.h
    COMMAND $<TARGET_FILE:protobuf::protoc>
        --proto_path=${PROTO_DIR}
        --cpp_out=${GEN_DIR}
        --grpc_out=${GEN_DIR}
        --plugin=protoc-gen-grpc=$<TARGET_FILE:gRPC::grpc_cpp_plugin>
        ${PROTO_FILE}
    DEPENDS ${PROTO_FILE}
)
```

这一步会生成四个关键文件：

- `hello.pb.h`
- `hello.pb.cc`
- `hello.grpc.pb.h`
- `hello.grpc.pb.cc`

它们分别负责：

- `hello.pb.*`：消息类型 `HelloRequest` / `HelloReply`
- `hello.grpc.pb.*`：服务接口 `Greeter::Service` 和客户端桩 `Greeter::Stub`

### 4.4 把生成代码打包成库

```cmake
add_library(hello-proto STATIC
    ${GEN_DIR}/hello.pb.cc
    ${GEN_DIR}/hello.grpc.pb.cc
)
target_include_directories(hello-proto PUBLIC ${GEN_DIR})
target_link_libraries(hello-proto PUBLIC protobuf::libprotobuf gRPC::grpc++)
```

这样做的好处是：客户端和服务端都复用同一份生成代码，不用各自重复配置。

## 5. 服务端是怎么写的

服务端源码在`hello-server/main.cpp`。

### 5.1 继承生成的服务基类

```cpp
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
```

这里发生了三件事：

- 继承 `Greeter::Service`
- 重写 `.proto` 中声明的 `SayHello`
- 从 `request` 读取参数，并把结果写入 `reply`

也就是说，`.proto` 定义的是“协议”，而这里写的是“具体业务实现”。

### 5.2 启动 gRPC 服务器

```cpp
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
```

这段代码可以按顺序理解：

1. 监听 `0.0.0.0:50051`
2. 注册 `GreeterServiceImpl`
3. 构建并启动服务器
4. 用 `Wait()` 阻塞主线程，持续对外提供 RPC 服务

这里使用的是 `InsecureServerCredentials()`，适合本地学习和内网测试。生产环境一般会换成 TLS/SSL 凭证。

## 6. 客户端是怎么写的

客户端源码在 `hello-client/main.cpp`。

### 6.1 创建连接通道

```cpp
std::string target_address("localhost:50051");
auto channel = grpc::CreateChannel(
    target_address,
    grpc::InsecureChannelCredentials()
);
```

这里的 `channel` 可以理解为客户端连接服务端的通信通道。

### 6.2 创建 Stub

```cpp
std::unique_ptr<Greeter::Stub> stub = Greeter::NewStub(channel);
```

`Stub` 可以理解为客户端侧的“代理对象”。

它的作用是：

- 暴露和服务端一致的方法名
- 把本地函数调用转换成真正的网络请求

### 6.3 发起 RPC 调用

```cpp
HelloRequest request;
request.set_name("World");

HelloReply reply;
ClientContext context;

Status status = stub->SayHello(&context, request, &reply);
```

这个流程很典型：

1. 构造请求对象 `request`
2. 设置请求参数 `name`
3. 准备响应对象 `reply`
4. 创建 `ClientContext`
5. 调用 `stub->SayHello(...)`

如果调用成功，服务端返回的内容就会写进 `reply`。

### 6.4 处理调用结果

```cpp
if (status.ok()) {
    std::cout << reply.message() << std::endl;
} else {
    std::cout << status.error_code() << ": "
              << status.error_message() << std::endl;
}
```

gRPC 调用结束后，要先检查 `status.ok()`：

- `true`：说明 RPC 调用成功
- `false`：说明连接、权限、超时或服务端处理等某个环节出错了

## 7. 一次完整调用的流程

这个示例从客户端到服务端的链路可以概括为：

1. 客户端创建 `HelloRequest`
2. 客户端调用 `stub->SayHello(...)`
3. gRPC 把请求序列化并通过网络发送给服务端
4. 服务端接收到请求后进入 `GreeterServiceImpl::SayHello`
5. 服务端读取 `request->name()`
6. 服务端写入 `reply->set_message(...)`
7. gRPC 把响应返回给客户端
8. 客户端从 `reply.message()` 里取出结果并打印

这就是一个最基础的 unary RPC，也就是“请求一次，响应一次”的调用模式。

## 8. 如何编译项目

如果你本机已经安装好 gRPC 和 protobuf，可以在项目根目录执行：

```bash
cmake -S . -B build
cmake --build build
```

当前项目已经存在 `build/` 目录和编译产物，所以你也可以直接重新增量构建：

```bash
cmake --build build
```

如果系统还没安装依赖，`CMakeLists.txt` 里已经给了 Ubuntu/Debian 方向的提示：

```bash
sudo apt install libgrpc++-dev protobuf-compiler-grpc
```

不同 Linux 发行版、macOS 或手动源码安装环境，包名可能略有差异。

这从内部调用 protocol buffer 编译器：

```shell
$ protoc -I ../../protos/ --grpc_out=. --plugin=protoc-gen-grpc=grpc_cpp_plugin ../../protos/helloworld.proto
$ protoc -I ../../protos/ --cpp_out=. ../../protos/helloworld.proto
```

## 9. 如何运行项目

先启动服务端：

```bash
./build/hello-server
```

再打开另一个终端运行客户端：

```bash
./build/hello-client
```

正常情况下，预期输出类似：

服务端：

```text
[Server] Listening on 0.0.0.0:50051
[Server] Received request from: World
```

客户端：

```text
[Client] Greeter replied: Hello, World
```

