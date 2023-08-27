add_library(NetworkSimulation STATIC NetworkSimulation.cpp Node.cpp)

target_include_directories(NetworkSimulation PUBLIC ${CMAKE_SOURCE_DIR}/src/io)

add_subdirectory(tests)
