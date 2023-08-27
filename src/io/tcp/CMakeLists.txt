add_subdirectory(tests)

add_library(Tcp SHARED Server.cpp Acceptor.cpp ClientManager.cpp Client.cpp)

