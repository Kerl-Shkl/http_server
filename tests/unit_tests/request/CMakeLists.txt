
function(create_request_test SOURCE_NAME)
    get_filename_component(TARGET_NAME ${SOURCE_NAME} NAME_WE)
    add_executable(${TARGET_NAME})
    target_sources(${TARGET_NAME} PRIVATE ${SOURCE_NAME})
    target_link_libraries(${TARGET_NAME} PRIVATE gtest gtest_main request)
    gtest_discover_tests(${TARGET_NAME})
endfunction()

create_request_test(method_parser_test.cpp)
create_request_test(headers_builder_test.cpp)
create_request_test(first_line_parser_test.cpp)
