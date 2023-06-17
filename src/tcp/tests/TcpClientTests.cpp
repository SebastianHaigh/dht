#include <catch2/catch_test_macros.hpp>

#include <iostream>

#include "../TcpClient.h"

TEST_CASE("TcpClient")
{

  tcp::TcpClient client{"127.0.0.1", 54000};
  std::cout << "TcpClient created" << std::endl;
  client.start();
  std::cout << "TcpClient started" << std::endl;

  client.stop();
  std::cout << "TcpClient stopped" << std::endl;
}
