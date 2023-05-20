#include <catch2/catch_test_macros.hpp>

#include "../ChordNode.h"

TEST_CASE("Test the creation of a chord node")
{
  ChordNode node = ChordNode{"200.178.0.1", 0};
  
  // Create another node and join the network that is currently formed by the first node
  
  ChordNode node2 = ChordNode{"200.178.0.5", 0};

  node2.join(node);

  REQUIRE(node.getSuccessorId() == node2.getPredecessorId());
  REQUIRE(node2.getPredecessorId() == node.getSuccessorId());

}
