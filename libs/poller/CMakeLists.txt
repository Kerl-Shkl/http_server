set(SOURCE_FILES src/poller.cpp)

add_library(poller ${SOURCE_FILES})
target_link_libraries(poller PRIVATE utils)
target_include_directories(poller PUBLIC include)
