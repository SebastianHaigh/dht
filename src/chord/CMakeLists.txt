add_library(Chord STATIC ChordNode.cpp FingerTable.cpp NodeId.cpp ChordMessaging.cpp)
target_link_libraries(Chord PRIVATE Hashing Tcp Async Comms)

enable_testing()

add_subdirectory(tests)
