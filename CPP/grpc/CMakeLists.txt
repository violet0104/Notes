cmake_minimum_required(VERSION 3.16)
project(grpc-hello LANGUAGES CXX)

# -------------------- 依赖查找 --------------------
# 方式一：系统安装的 gRPC（推荐 Ubuntu/Debian 用 apt 装好）
#   sudo apt install libgrpc++-dev protobuf-compiler-grpc
find_package(protobuf CONFIG REQUIRED)
find_package(gRPC CONFIG REQUIRED)

# -------------------- Proto 编译 --------------------
# 将 proto 文件编译为 .pb.cc / .pb.h / .grpc.pb.cc / .grpc.pb.h
set(PROTO_DIR "${CMAKE_CURRENT_SOURCE_DIR}/hello-server/proto")
set(PROTO_FILE "${PROTO_DIR}/hello.proto")

set(GEN_DIR  "${CMAKE_CURRENT_BINARY_DIR}/generated")
file(MAKE_DIRECTORY ${GEN_DIR})

# 生成消息类 (.pb.cc / .pb.h / .grpc.pb.cc / .grpc.pb.h)
# 使用生成器表达式 $<TARGET_FILE:...> 获取 protoc / grpc_cpp_plugin 路径
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
    COMMENT "Generating gRPC code from hello.proto"
)

# 将生成的文件打包为一个库，方便 server / client 复用
add_library(hello-proto STATIC
    ${GEN_DIR}/hello.pb.cc
    ${GEN_DIR}/hello.grpc.pb.cc
)
target_include_directories(hello-proto PUBLIC ${GEN_DIR})
target_link_libraries(hello-proto PUBLIC protobuf::libprotobuf gRPC::grpc++)

# -------------------- Server --------------------
add_executable(hello-server hello-server/main.cpp)
target_link_libraries(hello-server PRIVATE hello-proto)

# -------------------- Client --------------------
add_executable(hello-client hello-client/main.cpp)
target_link_libraries(hello-client PRIVATE hello-proto)
