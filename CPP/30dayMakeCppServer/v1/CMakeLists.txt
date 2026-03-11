cmake_minimum_required(VERSION 4.1)
project(CALC)

file(GLOB SRC ${PROJECT_SOURCE_DIR}/src/*.cpp)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
set(LIBRARY_OUTPUT_PATH /home/zdc/Notes/CPP/30dayMakeCppServer/test_cmake/lib)
# add_library(calc SHARED ${SRC})
# add_library(calc STATIC ${SRC})
add_executable(app ${SRC})
