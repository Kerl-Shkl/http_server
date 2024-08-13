cmake_minimum_required(VERSION 3.22)

project(Server VERSION 1.0)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(SOURCE_FILES src/main.cpp src/linux_connection.cpp src/epoll_connection.cpp src/request.cpp src/server.cpp
    src/logger.cpp src/response.cpp)

add_executable(exec ${SOURCE_FILES})
target_include_directories(exec PRIVATE include)

add_subdirectory(tests)
