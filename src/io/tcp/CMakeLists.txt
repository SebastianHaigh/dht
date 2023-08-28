add_subdirectory(tests)

add_library(Tcp SHARED Server.cpp Acceptor.cpp ClientManager.cpp Client.cpp)

target_include_directories(Tcp PRIVATE ${CMAKE_SOURCE_DIR}/src/io)

