set(SOURCE_FILES src/client_connection.cpp
                 src/listener.cpp src/server.cpp)

add_library(server ${SOURCE_FILES})
target_include_directories(server PUBLIC include)
target_link_libraries(server PUBLIC utils logical_controller poller)
