set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

enable_testing()

include(GoogleTest)

add_subdirectory(unit_tests)
add_subdirectory(unit_tests/request)
