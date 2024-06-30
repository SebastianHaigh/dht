add_library(NetworkSimulation STATIC Network.cpp Node.cpp)

target_link_libraries(NetworkSimulation PUBLIC Comms Boost::boost)

add_subdirectory(tests)
