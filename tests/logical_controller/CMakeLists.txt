add_executable(test_server echo_http_server.cpp)
target_link_libraries(test_server PRIVATE new_server)

find_package(Python REQUIRED)

add_test(NAME logical_controller_dummy_test
         COMMAND ${Python_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tester.py
                 --executable $<TARGET_FILE:test_server>)
