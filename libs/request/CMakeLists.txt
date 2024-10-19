set(SOURCE_FILES src/request_builder.cpp src/method_parser.cpp
    src/headers_builder.cpp src/first_line_parser.cpp src/request.cpp)

add_library(request ${SOURCE_FILES})
target_include_directories(request PUBLIC include)
