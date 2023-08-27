add_executable(NetworkSimulationTests NetworkSimulationTests.cpp)
target_link_libraries(NetworkSimulationTests PRIVATE Catch2::Catch2WithMain NetworkSimulation)
target_include_directories(NetworkSimulationTests PRIVATE ${CMAKE_SOURCE_DIR}/src/io/simulation)
add_test(NAME NetworkSimulationTests
         COMMAND NetworkSimulationTests)

