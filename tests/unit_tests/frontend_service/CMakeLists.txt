function(create_frontend_service_test SOURCE_NAME)
  get_filename_component(TARGET_NAME ${SOURCE_NAME} NAME_WE)
  add_executable(${TARGET_NAME})
  target_sources(${TARGET_NAME} PRIVATE ${SOURCE_NAME})
  target_link_libraries(${TARGET_NAME} PRIVATE gtest gtest_main
                                               frontend_service)
  gtest_discover_tests(${TARGET_NAME})
endfunction()

create_frontend_service_test(frontend_service_test.cpp)
create_frontend_service_test(resource_observer_test.cpp)
