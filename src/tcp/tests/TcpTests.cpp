#include <catch2/catch_test_macros.hpp>

#include <iostream>
#include <vector>

#include "../TcpServer.h"
#include "../TcpClient.h"

TEST_CASE("bob")
{
  // This test requires a TcpServer to connect to and a TcpClient to connect from.
  TcpServer server{"127.0.0.1", 54000};
  std::vector<std::unique_ptr<tcp::TcpClient>> clients;

  for (int i = 0; i < 10; ++i)
  {
    clients.push_back(std::make_unique<tcp::TcpClient>("127.0.0.1", 54000));
  }

  server.start();

  for (auto& client : clients)
  {
    client->start();
  }

  std::this_thread::sleep_for(std::chrono::seconds(5));

  for (auto& client : clients)
  {
    client->stop();
  }
  server.stop();
}

