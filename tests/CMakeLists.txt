set(gtest_force_shared_crt
    ON
    CACHE BOOL "" FORCE)

enable_testing()

include(GoogleTest)

add_subdirectory(unit_tests/request)
add_subdirectory(unit_tests/frontend_service)
add_subdirectory(unit_tests/poller)
add_subdirectory(unit_tests/permissions_controller)
add_subdirectory(logical_controller)
add_subdirectory(data_service)
