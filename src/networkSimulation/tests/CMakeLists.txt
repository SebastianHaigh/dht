add_executable(NetworkSimulationTests NetworkSimulationTests.cpp)
target_link_libraries(NetworkSimulationTests PRIVATE Catch2::Catch2WithMain NetworkSimulation)
add_test(NAME NetworkSimulationTests
         COMMAND NetworkSimulationTests)

