add_library(Chord STATIC ChordNode.cpp FingerTable.cpp NodeId.cpp)
target_link_libraries(Chord PRIVATE Hashing Tcp Async)

enable_testing()

add_subdirectory(tests)
