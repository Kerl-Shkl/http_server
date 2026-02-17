set(SOURCE_FILES src/database.cpp src/pq_database.cpp)

add_library(data_service ${SOURCE_FILES})

find_library(PQXX_LIB pqxx)
find_library(PQ_LIB pq)
target_link_libraries(data_service PRIVATE ${PQXX_LIB} ${PQ_LIB})
target_link_libraries(data_service PUBLIC utils)

target_include_directories(data_service PUBLIC include)

set(gtest_force_shared_crt
    ON
    CACHE BOOL "" FORCE)

enable_testing()

include(GoogleTest)

function(create_data_service_test SOURCE_NAME)
  get_filename_component(TARGET_NAME ${SOURCE_NAME} NAME_WE)
  add_executable(${TARGET_NAME})
  target_sources(${TARGET_NAME} PRIVATE ${SOURCE_NAME})
  target_link_libraries(${TARGET_NAME} PRIVATE gtest gtest_main data_service ${PQXX_LIB} ${PQ_LIB})
  target_include_directories(${TARGET_NAME} PRIVATE src)
  gtest_discover_tests(${TARGET_NAME})
endfunction()

create_data_service_test("tests/pq_utils_test.cpp")
