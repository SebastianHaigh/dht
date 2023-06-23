#include <catch2/catch_test_macros.hpp>
#include <iostream>
#include <memory>
#include "../NetworkSimulation.h"

TEST_CASE("Network simulation first test")
{
  // Create a network
  NetworkSimulator simulator;

  // Values to be received
  int receivedAtNode0{0};
  int receivedAtNode1{0};
  int receivedAtNode2{0};
  int receivedAtNode3{0};

  auto receiveHandlerForNode0 = [&receivedAtNode0] (int message) { receivedAtNode0 = message; };
  auto receiveHandlerForNode1 = [&receivedAtNode1] (int message) { receivedAtNode1 = message; };
  auto receiveHandlerForNode2 = [&receivedAtNode2] (int message) { receivedAtNode2 = message; };
  auto receiveHandlerForNode3 = [&receivedAtNode3] (int message) { receivedAtNode3 = message; };
  
  // Add two nodes to the network
  auto& node0 = simulator.addNode(0, receiveHandlerForNode0);
  auto& node1 = simulator.addNode(1, receiveHandlerForNode1);

  // Send a message from node a and check it is received on node b
  node0.sendMessage(1, 1);
  
  CHECK(receivedAtNode1 == 1);

  // Send a message from node b and check it is received on node a
  node1.sendMessage(0, 2);

  CHECK(receivedAtNode0 == 2);

  auto& node2 = simulator.addNode(2, receiveHandlerForNode2);
  auto& node3 = simulator.addNode(3, receiveHandlerForNode3);

  for (int i = 0; i < 4; i++)
  {
    node2.sendMessage(i, i);
  }

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
