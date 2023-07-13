enable_testing()

add_executable(ChordTests ChordTests.cpp)
target_link_libraries(ChordTests 
                      PRIVATE 
                      Catch2::Catch2WithMain 
                      Chord
                      NetworkSimulation)
add_test(NAME ChordTests
         COMMAND ChordTests)

add_executable(FingerTableTests FingerTableTests.cpp)
target_link_libraries(FingerTableTests 
                      PRIVATE 
                      Catch2::Catch2WithMain 
                      Chord)
add_test(NAME FingerTableTests
         COMMAND FingerTableTests)
