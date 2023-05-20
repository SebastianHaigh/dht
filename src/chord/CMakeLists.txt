add_library(Chord STATIC ChordNode.cpp)
target_link_libraries(Chord PRIVATE Hashing)

add_subdirectory(tests)
