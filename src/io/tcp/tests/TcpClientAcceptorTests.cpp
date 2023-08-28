#include <catch2/catch_test_macros.hpp>

#include <tcp/Acceptor.h>
#include <tcp/ClientManager.h>

namespace odd::io::tcp {

TEST_CASE("TcpAcceptor")
{
  ClientManager manager;
  Acceptor acceptor{"127.0.0.1", 54000, &manager};
}

} // namespace odd::io::tcp

