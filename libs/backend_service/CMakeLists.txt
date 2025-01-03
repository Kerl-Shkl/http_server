set(SOURCE_FILES src/backend_service.cpp)

add_library(backend_service ${SOURCE_FILES})
target_link_libraries(backend_service PUBLIC logical_controller
                                             frontend_service)
target_include_directories(backend_service PUBLIC include)
