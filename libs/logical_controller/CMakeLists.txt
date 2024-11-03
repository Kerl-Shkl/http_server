set(SOURCE_FILES src/logical_controller.cpp)

add_library(logical_controller ${SOURCE_FILES})
target_include_directories(logical_controller PUBLIC include)
target_link_libraries(logical_controller PUBLIC request)
