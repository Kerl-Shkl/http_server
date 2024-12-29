set(SOURCE_FILES src/frontend_service.cpp)

add_library(frontend_service ${SOURCE_FILES})
target_link_libraries(frontend_service PUBLIC poller)
target_include_directories(frontend_service PUBLIC include)
