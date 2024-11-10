set(SOURCE_FILES src/response.cpp)

add_library(response ${SOURCE_FILES})
target_include_directories(response PUBLIC include)
