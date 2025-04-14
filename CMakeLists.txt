cmake_minimum_required(VERSION 3.22)

project(Server VERSION 1.0)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_subdirectory(third_party)
add_subdirectory(libs)

set(SOURCE_FILES src/main.cpp src/startup.cpp)

if(POLICY CMP0167)
  cmake_policy(SET CMP0167 NEW)
endif()
find_package(
  Boost
  COMPONENTS program_options
  REQUIRED)

add_executable(exec ${SOURCE_FILES})
target_link_libraries(exec PRIVATE ${Boost_LIBRARIES} server backend_service)
target_include_directories(exec PRIVATE include)

add_subdirectory(tests)
