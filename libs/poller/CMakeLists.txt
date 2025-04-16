set(SOURCE_FILES src/poller.cpp src/linux_connection.cpp)

add_library(poller ${SOURCE_FILES})
target_link_libraries(poller PUBLIC utils)
target_include_directories(poller PUBLIC include)
