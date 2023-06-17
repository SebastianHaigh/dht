add_library(Tcp SHARED TcpServer.cpp TcpAcceptor.cpp TcpClientManager.cpp TcpClient.cpp)

add_subdirectory(tests)
