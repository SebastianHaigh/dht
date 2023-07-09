#include <catch2/catch_test_macros.hpp>

#include <cstdint>
#include <memory>
#include <thread>
#include <bit>

#include "../ChordNode.h"
#include "../ChordMessaging.h"
#include "../NodeId.h"

namespace chord { namespace test {
class Timer {
  private:
    std::chrono::time_point<std::chrono::high_resolution_clock> start_timepoint;
  public:
    Timer()
    {
      start_timepoint = std::chrono::high_resolution_clock::now();
    }

    ~Timer()
    {
      stop();
    }

    void stop()
    {

      auto end_timepoint = std::chrono::high_resolution_clock::now();
      auto start = std::chrono::time_point_cast<std::chrono::microseconds>(start_timepoint).time_since_epoch().count();
      auto stop = std::chrono::time_point_cast<std::chrono::microseconds>(end_timepoint).time_since_epoch().count();

      auto duration = stop - start;

      std::cout << duration << " us" << std::endl;
    }
};

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

  hashing::SHA1Hash twoToTheZeroHash = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x1 };
  hashing::SHA1Hash twoToTheOneHash = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x2 };
  hashing::SHA1Hash twoToTheTwoHash = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x4 };
  hashing::SHA1Hash twoToTheThreeHash = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x8 };
  hashing::SHA1Hash twoToTheFourHash = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x10 };
  hashing::SHA1Hash twoToTheFiveHash = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x20 };
  hashing::SHA1Hash twoToTheSixHash = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x40 };
  hashing::SHA1Hash twoToTheSevenHash = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x80 };
  hashing::SHA1Hash twoToTheEightHash = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x1, 0 };

  REQUIRE(twoToTheZero == NodeId{twoToTheZeroHash});
  REQUIRE(twoToTheOne == NodeId{twoToTheOneHash});
  REQUIRE(twoToTheTwo == NodeId(twoToTheTwoHash));
  REQUIRE(twoToTheThree == NodeId(twoToTheThreeHash));
  REQUIRE(twoToTheFour == NodeId(twoToTheFourHash));
  REQUIRE(twoToTheFive == NodeId(twoToTheFiveHash));
  REQUIRE(twoToTheSix == NodeId(twoToTheSixHash));
  REQUIRE(twoToTheSeven == NodeId(twoToTheSevenHash));
  REQUIRE(twoToTheEight == NodeId(twoToTheEightHash));

}

TEST_CASE("Add some more node ids")
{
  NodeId nodeId_0, nodeId_1, expected;

  SECTION("Case 1")
  {
    nodeId_0 = NodeId{ "FFFFFFFF-FFFFFFFF-FFFFFFFF-FFFFFFFF-FFFFFFFF" };
    nodeId_1 = NodeId{ "00000000-00000100-00000000-00000000-00000000" };
    expected = NodeId{ "00000000-000000FF-FFFFFFFF-FFFFFFFF-FFFFFFFF" };
  }

  SECTION("Case 2")
  {
    nodeId_0 = NodeId{ "FFFFFFFF-FFFFFFFF-FFFFFFFF-FFFFFFFF-FFFFFFFF" };
    nodeId_1 = NodeId{ "FFFFFFFF-FFFFFFFF-FFFFFFFF-FFFFFFFF-FFFFFFFF" };
    expected = NodeId{ "FFFFFFFF-FFFFFFFF-FFFFFFFF-FFFFFFFF-FFFFFFFE" };
  }

  auto result = nodeId_0 + nodeId_1;

  REQUIRE(result == expected);
}

TEST_CASE("Test the creation of a chord node")
{
  ChordNode node{"200.178.0.1", 0};
  
  // Create another node and join the network that is currently formed by the first node
  
  ChordNode node2{"200.178.0.5", 0};

  node2.join("200.178.0.1");
  
  REQUIRE(node.getSuccessorId() == node2.getPredecessorId());
  REQUIRE(node2.getPredecessorId() == node.getSuccessorId());
}

TEST_CASE("Chord messaging test")
{
  NodeId nodeId { "12345678-abcdabcd-effeeffe-dcbadcba-87654321" };
  NodeId sourceNodeId { "87654321-abcdabcd-eff00ffe-dcbadcba-12345678" };

  FindSuccessorMessage message{CommsVersion::V1, nodeId, sourceNodeId};

  EncodedMessage encoded = message.encode();

  FindSuccessorMessage decodedMessage{CommsVersion::V1};

  decodedMessage.decode(encoded);

  REQUIRE(decodedMessage.queryNodeId() == nodeId);
  REQUIRE(decodedMessage.sourceNodeId() == sourceNodeId);
}

}} // namespace chord::test

