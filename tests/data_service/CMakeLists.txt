function(create_data_service_test SOURCE_NAME)
  get_filename_component(TARGET_NAME ${SOURCE_NAME} NAME_WE)
  add_executable(${TARGET_NAME})
  target_sources(${TARGET_NAME} PRIVATE ${SOURCE_NAME})
  target_link_libraries(${TARGET_NAME} PRIVATE gtest gtest_main data_service)
  gtest_discover_tests(${TARGET_NAME})
endfunction()

# database_test need link with pqxx and pq
add_executable(database_test database_test.cpp)
find_library(PQXX_LIB pqxx)
find_library(PQ_LIB pq)
target_link_libraries(database_test PRIVATE gtest gtest_main data_service
                                            ${PQXX_LIB} ${PQ_LIB})
gtest_discover_tests(database_test)
