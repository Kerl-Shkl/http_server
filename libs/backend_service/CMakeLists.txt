set(SOURCE_FILES src/backend_service.cpp)

add_library(backend_service ${SOURCE_FILES})
target_link_libraries(
  backend_service
  PUBLIC permissions_controller data_service server
         nlohmann_json::nlohmann_json md4c-html)
target_include_directories(backend_service PUBLIC include)
target_include_directories(
  backend_service PRIVATE ${CMAKE_SOURCE_DIR}/third_party/md4c/md2html
                          ${CMAKE_SOURCE_DIR}/third_party/md4c/src)
