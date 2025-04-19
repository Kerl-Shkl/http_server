set(SOURCE_FILES src/permissions_controller.cpp src/bot_communicator.cpp)

add_library(permissions_controller ${SOURCE_FILES})
target_link_libraries(permissions_controller PUBLIC stduuid utils poller)
target_include_directories(permissions_controller PUBLIC include)
