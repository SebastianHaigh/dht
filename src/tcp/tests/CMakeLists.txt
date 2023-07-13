add_executable(TcpTests TcpTests.cpp)
target_link_libraries(TcpTests PRIVATE Catch2::Catch2WithMain Tcp)
add_test(NAME TcpTests
         COMMAND TcpTests)

add_executable(TcpClientAcceptorTests TcpClientAcceptorTests.cpp)
target_link_libraries(TcpClientAcceptorTests PRIVATE Catch2::Catch2WithMain Tcp)
add_test(NAME TcpClientAcceptorTests
         COMMAND TcpClientAcceptorTests)
