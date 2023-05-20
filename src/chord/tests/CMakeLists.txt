add_executable(ChordTests ChordTests.cpp)
target_link_libraries(ChordTests PRIVATE Catch2::Catch2WithMain Chord Hashing)
add_test(NAME ChordTests
         COMMAND ChordTests)
