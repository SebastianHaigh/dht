#include <catch2/catch_test_macros.hpp>

#include "../TcpAcceptor.h"
#include "../TcpClientManager.h"

TEST_CASE("TcpAcceptor")
{
  TcpClientManager manager;
  TcpClientAcceptor acceptor{"127.0.0.1", 54000, &manager};
}
