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
  
  auto receiveHandlerForNode0 = [&receivedAtNode0] (int message) { receivedAtNode0 = message; };
  auto receiveHandlerForNode1 = [&receivedAtNode1] (int message) { receivedAtNode1 = message; };
  
  // Add two nodes to the network
  auto& node0 = simulator.addNode(1, receiveHandlerForNode0);
  auto& node1 = simulator.addNode(2, receiveHandlerForNode1);

  // Send a message from node a and check it is received on node b
  node0.sendMessage(2, 1);
  
  CHECK(receivedAtNode1 == 1);

  // Send a message from node b and check it is received on node a
  node1.sendMessage(1, 2);

  CHECK(receivedAtNode0 == 2);
}

TEST_CASE("Network simulator verifies the IP addresses provided are valid")
{

}

TEST_CASE("Network simulator does not allow two nodes with the same address to be added")
{
}
