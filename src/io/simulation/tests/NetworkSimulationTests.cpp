#include <boost/asio/ip/address.hpp>
#include <catch2/catch_test_macros.hpp>
#include <cstdint>
#include <iostream>
#include <memory>
#include "EncodedMessage.h"

#include <Network.h>

namespace odd::io::simulation {

TEST_CASE("Network simulation first test")
{
  // Create a network
  Network simulator;

  // Values to be received
  int receivedAtNode0{0};
  int receivedAtNode1{0};
  int receivedAtNode2{0};
  int receivedAtNode3{0};

  using Ip = boost::asio::ip::address;
  auto receiveHandlerForNode0 = [&receivedAtNode0] (const Ip& sourceIp, const EncodedMessage& message) { receivedAtNode0 = static_cast<int>(message.length()); };
  auto receiveHandlerForNode1 = [&receivedAtNode1] (const Ip& sourceIp, const EncodedMessage& message) { receivedAtNode1 = static_cast<int>(message.length()); };
  auto receiveHandlerForNode2 = [&receivedAtNode2] (const Ip& sourceIp, const EncodedMessage& message) { receivedAtNode2 = static_cast<int>(message.length()); };
  auto receiveHandlerForNode3 = [&receivedAtNode3] (const Ip& sourceIp, const EncodedMessage& message) { receivedAtNode3 = static_cast<int>(message.length()); };

  std::vector<Ip> nodeIps = {
    boost::asio::ip::make_address("127.0.10.0"),
    boost::asio::ip::make_address("127.0.10.1"),
    boost::asio::ip::make_address("127.0.10.2"),
    boost::asio::ip::make_address("127.0.10.3")
  };

  // Add two nodes to the network
  auto& node0 = simulator.addNode(nodeIps[0], receiveHandlerForNode0);
  auto& node1 = simulator.addNode(nodeIps[1], receiveHandlerForNode1);

  uint8_t msgOctets[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

  // Send a message from node a and check it is received on node b
  EncodedMessage msg1{ msgOctets, 1 };
  node0.sendMessage(nodeIps[1], msg1);

  simulator.run();

  CHECK(receivedAtNode1 == 1);

  // Send a message from node b and check it is received on node a
  EncodedMessage msg2{ msgOctets, 2 };
  node1.sendMessage(nodeIps[0], msg2);
  simulator.run();

  CHECK(receivedAtNode0 == 2);

  auto& node2 = simulator.addNode(nodeIps[2], receiveHandlerForNode2);
  auto& node3 = simulator.addNode(nodeIps[3], receiveHandlerForNode3);

  for (size_t i = 0; i < 4; i++)
  {
    EncodedMessage msg{ msgOctets, i };
    node2.sendMessage(nodeIps[i], msg);
  }
  simulator.run();

  CHECK(receivedAtNode0 == 0);
  CHECK(receivedAtNode1 == 1);
  CHECK(receivedAtNode2 == 2);
  CHECK(receivedAtNode3 == 3);

}

TEST_CASE("Network simulator verifies the IP addresses provided are valid")
{
}

TEST_CASE("Network simulator does not allow two nodes with the same address to be added")
{
}

} // namespace odd::io::simulation

