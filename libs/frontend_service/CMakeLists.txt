set(SOURCE_FILES src/frontend_service.cpp)

add_library(frontend_service ${SOURCE_FILES})
target_include_directories(frontend_service PUBLIC include)
