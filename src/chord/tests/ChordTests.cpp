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

TEST_CASE("Check the speed of the addition")
{

  if (std::endian::native == std::endian::little)
  std::cout << "little endian" << std::endl;
  else
    std::cout << "not little" << std::endl;

  uint32_t hashvals[5] = { 0xFFFFFFFF, 
                           0xFFFFFFFF, 
                           0xFFFFFFFF,
                           0xFFFFFFFF, 
                           0xFFFFFFFF };
  NodeId largeNodeId{ hashvals };
  NodeId anotherNodeId{ hashing::SHA1Hash{ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                                           0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                                           0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                                           0xFF, 0xFF, 0xFF, 0xFF, 0xFF } };
  {
    Timer timer;
    for (int i = 0; i < 220000; i++)
    {
      auto node = largeNodeId + anotherNodeId;
    }
  }

  REQUIRE(false);
}

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

  std::vector<NodeId> nodeIds = { NodeId::powerOfTwo(0),
                                  NodeId::powerOfTwo(1),
                                  NodeId::powerOfTwo(2),
                                  NodeId::powerOfTwo(3),
                                  NodeId::powerOfTwo(4),
                                  NodeId::powerOfTwo(5),
                                  NodeId::powerOfTwo(6),
                                  NodeId::powerOfTwo(7),
                                  NodeId::powerOfTwo(8) };

  hashing::SHA1Hash twoToTheZeroHash = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x1 };
  hashing::SHA1Hash twoToTheOneHash = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x2 };
  hashing::SHA1Hash twoToTheTwoHash = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x4 };
  hashing::SHA1Hash twoToTheThreeHash = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x8 };
  hashing::SHA1Hash twoToTheFourHash = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x10 };
  hashing::SHA1Hash twoToTheFiveHash = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x20 };
  hashing::SHA1Hash twoToTheSixHash = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x40 };
  hashing::SHA1Hash twoToTheSevenHash = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x80 };
  hashing::SHA1Hash twoToTheEightHash = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x1, 0 };

  std::vector<unsigned char*> hashes{ &twoToTheZeroHash[0], 
                                        &twoToTheOneHash[0], 
                                         &twoToTheTwoHash[0],
                                         &twoToTheThreeHash[0],
                                         &twoToTheFourHash[0],
                                         &twoToTheFiveHash[0],
                                         &twoToTheSixHash[0],
                                         &twoToTheSevenHash[0],
                                         &twoToTheEightHash[0] };

  int counter{ 0 };
  for (auto& hash : hashes)
  {
    std::cout << std::dec << counter << ": " << nodeIds[counter].toString() << ", hash version " << NodeId{hash}.toString() << std::endl;
    counter++;
  } 


  REQUIRE(twoToTheZero == NodeId{twoToTheZeroHash});
  REQUIRE(twoToTheOne == NodeId{twoToTheOneHash});
  REQUIRE(twoToTheTwo == NodeId(twoToTheTwoHash));
  REQUIRE(twoToTheThree == NodeId(twoToTheThreeHash));
  REQUIRE(twoToTheFour == NodeId(twoToTheFourHash));
  REQUIRE(twoToTheFive == NodeId(twoToTheFiveHash));
  REQUIRE(twoToTheSix == NodeId(twoToTheSixHash));
  REQUIRE(twoToTheSeven == NodeId(twoToTheSevenHash));
  REQUIRE(twoToTheEight == NodeId(twoToTheEightHash));

  // Large number addition
  hashing::SHA1Hash largeHashValue = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                                       0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                                       0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                                       0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

  
  NodeId largeNodeId{ hashing::SHA1Hash{ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                                         0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                                         0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                                         0xFF, 0xFF, 0xFF, 0xFF, 0xFF } };
  NodeId anotherNodeId{ hashing::SHA1Hash{ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                                           0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                                           0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                                           0xFF, 0xFF, 0xFF, 0xFF, 0xFF } };

  uint16_t toBeAddedTo = 0xFFFF;
  uint16_t toAdd = 0xFFFF;
  uint16_t theSum = toBeAddedTo + toAdd;

  std::cout << std::hex << toBeAddedTo << " + " << std::dec << toAdd << " = " << std::hex << theSum << std::endl;

  for (int i = 0; i < 5000; i++)
  {
    std::cout << largeNodeId.toString() << std::endl;

    std::this_thread::sleep_for(std::chrono::milliseconds{500});
    largeNodeId = largeNodeId + anotherNodeId;
  }

  
  
  
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

TEST_CASE("Chord messaging test")
{
  NodeId nodeId { "12345678-abcdabcd-effeeffe-dcbadcba-87654321" };

  FindSuccessorMessage message{CommsVersion::V1, nodeId};

  EncodedMessage encoded = message.encode();

  FindSuccessorMessage decodedMessage{CommsVersion::V1};

  decodedMessage.decode(encoded);

  auto decodedNodeId = decodedMessage.nodeId();

  REQUIRE(decodedNodeId == nodeId);
}

}} // namespace chord::test

