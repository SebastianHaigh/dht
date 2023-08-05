#include <catch2/catch_test_macros.hpp>
#include "../RPCs.h"

TEST_CASE("Create and join two test nodes")
{
  // Create node configs
  NodeConfig nodeConfig1, nodeConfig2;
  nodeConfig1.m_id = 1;
  nodeConfig1.m_neighbourIpAddresses[0] = 2;
  nodeConfig2.m_id = 2;
  nodeConfig2.m_neighbourIpAddresses[0] = 1;

  NetworkSimulator network;  

  auto& node1 = network.addNode(1);
  auto& node2 = network.addNode(2);

  Node testNode1{node1, nodeConfig1};
  Node testNode2{node2, nodeConfig2};
}

