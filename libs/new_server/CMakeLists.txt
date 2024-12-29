set(SOURCE_FILES src/linux_connection.cpp src/poller.cpp
                 src/client_connection.cpp src/listener.cpp src/server.cpp)

add_library(new_server ${SOURCE_FILES})
target_include_directories(new_server PUBLIC include)
target_link_libraries(new_server PUBLIC utils logical_controller)
