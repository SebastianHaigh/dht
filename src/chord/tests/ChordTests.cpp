#include <catch2/catch_test_macros.hpp>

#include "../ChordNode.h"
#include "../NodeId.h"

namespace chord { namespace test {

TEST_CASE("Test the creation of a node id")
{
  NodeId twoToTheZero = NodeId::powerOfTwo(0);
  NodeId twoToTheOne = NodeId::powerOfTwo(1);
  NodeId twoToTheTwo = NodeId::powerOfTwo(2);
  NodeId twoToTheThree = NodeId::powerOfTwo(3);
  NodeId twoToTheFour = NodeId::powerOfTwo(4);
  NodeId twoToTheFive = NodeId::powerOfTwo(5);
  NodeId twoToTheSix = NodeId::powerOfTwo(6);
  NodeId twoToTheSeven = NodeId::powerOfTwo(7);
  NodeId twoToTheEight = NodeId::powerOfTwo(8);

  hashing::SHA1Hash twoToTheZeroHash = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x80 };
  hashing::SHA1Hash twoToTheOneHash = { 0x40, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
  hashing::SHA1Hash twoToTheTwoHash = { 0x20, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
  hashing::SHA1Hash twoToTheThreeHash = { 0x10, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
  hashing::SHA1Hash twoToTheFourHash = { 0x8, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
  hashing::SHA1Hash twoToTheFiveHash = { 0x4, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
  hashing::SHA1Hash twoToTheSixHash = { 0x2, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
  hashing::SHA1Hash twoToTheSevenHash = { 0x1, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
  hashing::SHA1Hash twoToTheEightHash = { 0x80, 0x80, 0, 0, 0, 0, 0, 0, 0, 0 };

  std::cout << std::hex << "twoToTheZero = " << twoToTheZero.toString() << std::endl;
  std::cout << std::hex << "twoToTheOne = " << twoToTheOne.toString() << std::endl;
  std::cout << std::hex << "twoToTheTwo = " << twoToTheTwo.toString() << std::endl;
  std::cout << std::hex << "twoToTheThree = " << twoToTheThree.toString() << std::endl;
  std::cout << std::hex << "twoToTheFour = " << twoToTheFour.toString() << std::endl;

  std::cout << std::hex << twoToTheZeroHash << std::endl;

  REQUIRE(twoToTheZero == NodeId{twoToTheZeroHash});
  REQUIRE(twoToTheOne == NodeId(twoToTheOneHash));
  REQUIRE(twoToTheTwo == NodeId(twoToTheTwoHash));
  REQUIRE(twoToTheThree == NodeId(twoToTheThreeHash));
  REQUIRE(twoToTheFour == NodeId(twoToTheFourHash));
  REQUIRE(twoToTheFive == NodeId(twoToTheFiveHash));
  REQUIRE(twoToTheSix == NodeId(twoToTheSixHash));
  REQUIRE(twoToTheSeven == NodeId(twoToTheSevenHash));
  REQUIRE(twoToTheEight == NodeId(twoToTheEightHash));

}

TEST_CASE("Test the creation of a chord node")
{
  std::cout << "Create first node" << std::endl;
  ChordNode node{"200.178.0.1", 0};
  
  // Create another node and join the network that is currently formed by the first node
  
  std::cout << "Create second node" << std::endl;
  ChordNode node2{"200.178.0.5", 0};

  std::cout << "Join second node to first node" << std::endl;
  node2.join("200.178.0.1");
  



  REQUIRE(node.getSuccessorId() == node2.getPredecessorId());
  REQUIRE(node2.getPredecessorId() == node.getSuccessorId());
}

}} // namespace chord::test

