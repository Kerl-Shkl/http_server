set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

enable_testing()

include(GoogleTest)

function(create_dummy_test SOURCE_NAME)
    get_filename_component(TARGET_NAME ${SOURCE_NAME} NAME_WE)
    add_executable(${TARGET_NAME})
    target_sources(${TARGET_NAME} PRIVATE ${SOURCE_NAME})
    target_include_directories(${TARGET_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/include)
    target_link_libraries(${TARGET_NAME} PRIVATE gtest gtest_main)
    gtest_discover_tests(${TARGET_NAME})
endfunction()

create_dummy_test(connections_list_test.cpp)
