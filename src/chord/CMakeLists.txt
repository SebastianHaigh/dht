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

enable_testing()

add_subdirectory(tests)
