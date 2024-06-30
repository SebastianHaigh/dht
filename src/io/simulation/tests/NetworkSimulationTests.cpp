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

  auto receiveHandlerForNode0 = [&receivedAtNode0] (uint32_t sourceIp, const EncodedMessage& message) { receivedAtNode0 = static_cast<int>(message.length()); };
  auto receiveHandlerForNode1 = [&receivedAtNode1] (uint32_t sourceIp, const EncodedMessage& message) { receivedAtNode1 = static_cast<int>(message.length()); };
  auto receiveHandlerForNode2 = [&receivedAtNode2] (uint32_t sourceIp, const EncodedMessage& message) { receivedAtNode2 = static_cast<int>(message.length()); };
  auto receiveHandlerForNode3 = [&receivedAtNode3] (uint32_t sourceIp, const EncodedMessage& message) { receivedAtNode3 = static_cast<int>(message.length()); };

  // Add two nodes to the network
  auto& node0 = simulator.addNode(0, receiveHandlerForNode0);
  auto& node1 = simulator.addNode(1, receiveHandlerForNode1);

  uint8_t msgOctets[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

  // Send a message from node a and check it is received on node b
  EncodedMessage msg1{ msgOctets, 1 };
  node0.sendMessage(1, msg1);

  simulator.run();

  CHECK(receivedAtNode1 == 1);

  // Send a message from node b and check it is received on node a
  EncodedMessage msg2{ msgOctets, 2 };
  node1.sendMessage(0, msg2);
  simulator.run();

  CHECK(receivedAtNode0 == 2);

  auto& node2 = simulator.addNode(2, receiveHandlerForNode2);
  auto& node3 = simulator.addNode(3, receiveHandlerForNode3);

  for (size_t i = 0; i < 4; i++)
  {
    EncodedMessage msg{ msgOctets, i };
    node2.sendMessage(i, msg);
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

