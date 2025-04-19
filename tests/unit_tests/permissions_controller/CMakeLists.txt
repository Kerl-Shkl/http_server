function(create_permissions_test SOURCE_NAME)
  get_filename_component(TARGET_NAME ${SOURCE_NAME} NAME_WE)
  add_executable(${TARGET_NAME})
  target_sources(${TARGET_NAME} PRIVATE ${SOURCE_NAME})
  target_link_libraries(${TARGET_NAME} PRIVATE gtest gtest_main
                                               permissions_controller)
  gtest_discover_tests(${TARGET_NAME})
endfunction()

create_permissions_test(bot_communicator_test.cpp)
