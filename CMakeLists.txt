cmake_minimum_required(VERSION 3.22)

project(Server VERSION 1.0)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_subdirectory(libs)

set(SOURCE_FILES src/main.cpp)

add_executable(exec ${SOURCE_FILES})
target_link_libraries(exec PRIVATE new_server)
target_include_directories(exec PRIVATE include)

add_subdirectory(tests)
