add_library(NetworkSimulation STATIC Network.cpp Node.cpp)

target_link_libraries(NetworkSimulation PUBLIC Comms)

add_subdirectory(tests)
