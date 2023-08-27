enable_testing()

add_executable(ChordTests ChordTests.cpp)
target_link_libraries(ChordTests 
                      PRIVATE 
                      Catch2::Catch2WithMain 
                      Chord
                      NetworkSimulation
                      Logging)
target_include_directories(ChordTests PRIVATE ${CMAKE_SOURCE_DIR}/src/io)
add_test(NAME ChordTests
         COMMAND ChordTests)
