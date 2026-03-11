cmake_minimum_required(VERSION 3.10)
project(CALC)
# 搜索指定目录下源文件
file(GLOB SRC_LIST ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
# 包含头文件路径
include_directories(${PROJECT_SOURCE_DIR}/include)
# 包含静态库路径
link_directories(${PROJECT_SOURCE_DIR}/lib2)
# # 链接静态库
# link_libraries(calc)
message("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx")
add_executable(app ${SRC_LIST})
message(STATUS "111xxxxxxxxxxxxxxxxxxxxxxxxxxx")
# 链接动态库
target_link_libraries(app calc)
message(FATAL_ERROR "222xxxxxxxxxxxxxxxxxxxxxx")
message(STATUS "333xxxxxxxxxxxxxxxxxxxxxxxxxxx")
