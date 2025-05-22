set(SOURCE_FILES src/database.cpp)

add_library(data_service ${SOURCE_FILES})

find_library(PQXX_LIB pqxx)
find_library(PQ_LIB pq)
target_link_libraries(data_service PRIVATE ${PQXX_LIB} ${PQ_LIB})

target_include_directories(data_service PUBLIC include)
