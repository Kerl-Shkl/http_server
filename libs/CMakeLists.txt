add_subdirectory(utils)
add_subdirectory(poller)
add_subdirectory(request)
add_subdirectory(response)
add_subdirectory(logical_controller)
add_subdirectory(backend_service)
add_subdirectory(server)
add_subdirectory(frontend_service)
add_subdirectory(data_service)

set(JSON_BuildTests
    OFF
    CACHE INTERNAL "")
add_subdirectory(nlohmann_json)
