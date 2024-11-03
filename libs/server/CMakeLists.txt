set(SOURCE_FILES src/server.cpp src/linux_connection.cpp src/epoll_connection.cpp src/client_connection.cpp)

add_library(server ${SOURCE_FILES})
target_include_directories(server PUBLIC include)
target_link_libraries(server PUBLIC utils request)
