#include <catch2/catch_test_macros.hpp>

#include <iostream>
#include <vector>

#include <tcp/Server.h>
#include <tcp/Client.h>

namespace odd::io::tcp {

TEST_CASE("First test to check client/server behaviour")
{
  // This test requires a TcpServer to connect to and a TcpClient to connect from.
  Server server{"127.0.0.1", 54000};
  std::vector<std::unique_ptr<Client>> clients;

  for (int i = 0; i < 10; ++i)
  {
    clients.push_back(std::make_unique<Client>("127.0.0.1", 54000));
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

} // namespace odd

