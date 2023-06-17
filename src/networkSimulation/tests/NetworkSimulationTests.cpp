#include <catch2/catch_test_macros.hpp>
#include <iostream>
#include <memory>
#include "../NetworkSimulation.h"

TEST_CASE("Network simulation first test")
{
  // Create a network
  NetworkSimulator simulator;
  
  // Add two nodes to the network
  auto& node0 = simulator.addNode(1);
  auto& node1 = simulator.addNode(2);

  // Send a message from node a and check it is received on node b
  

  // Send a message from node b and check it is received on node a
}

TEST_CASE("Network simulator verifies the IP addresses provided are valid")
{
  std::vector<SimulatedNode> nodes = { {0, 0}, {1, 1}, {2, 2} };

  SimulatedNode* n0_p = &nodes[0];
  SimulatedNode* n1_p = &nodes[1];
  SimulatedNode* n2_p = &nodes[2];

  CHECK(n0_p->nodeId() == 0);
  CHECK(n1_p->nodeId() == 1);
  CHECK(n2_p->nodeId() == 2);

  auto it = nodes.begin();
  it++;

  nodes.erase(it);

  CHECK(nodes.size() == 2);

  CHECK(n0_p->nodeId() == 0);
  CHECK(n1_p->nodeId() == 2);
  CHECK(n2_p->nodeId() == 2);

  std::vector<std::unique_ptr<SimulatedNode>> node_ptrs; 
  node_ptrs.push_back(std::make_unique<SimulatedNode>(0, 0));
  node_ptrs.push_back(std::make_unique<SimulatedNode>(1, 1));
  node_ptrs.push_back(std::make_unique<SimulatedNode>(2, 2));

  SimulatedNode* node0_p = node_ptrs[0].get();
  SimulatedNode* node1_p = node_ptrs[1].get();
  SimulatedNode* node2_p = node_ptrs[2].get();

  auto node_it = node_ptrs.begin();
  node_it++;

  node_ptrs.erase(node_it);

  CHECK(nodes.size() == 2);

  CHECK(node0_p->nodeId() == 0);
  CHECK(node2_p->nodeId() == 2);
}

TEST_CASE("Network simulator does not allow two nodes with the same address to be added")
{
}
