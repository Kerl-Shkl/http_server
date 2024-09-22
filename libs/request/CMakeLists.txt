set(SOURCE_FILES src/request_builder.cpp src/method_parser.cpp)

add_library(request ${SOURCE_FILES})
target_include_directories(request PUBLIC include)
