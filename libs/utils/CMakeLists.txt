set(SOURCE_FILES src/logger.cpp)

add_library(utils ${SOURCE_FILES})
target_include_directories(utils PUBLIC include)
