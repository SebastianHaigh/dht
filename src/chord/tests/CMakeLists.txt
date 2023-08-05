enable_testing()

add_executable(ChordTests ChordTests.cpp)
target_link_libraries(ChordTests 
                      PRIVATE 
                      Catch2::Catch2WithMain 
                      Chord
                      NetworkSimulation
                      Logging)
add_test(NAME ChordTests
         COMMAND ChordTests)
