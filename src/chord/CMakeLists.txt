add_library(Chord
            STATIC
            ChordNode.cpp
            FingerTable.cpp
            NodeId.cpp
            ChordMessaging.cpp
            ConnectionManager.cpp)
target_link_libraries(Chord
                      PRIVATE
                      Hashing
                      Tcp
                      Async
                      Comms
                      Logging)

target_include_directories(Chord PRIVATE ${CMAKE_SOURCE_DIR}/src/io)

enable_testing()

add_subdirectory(tests)
