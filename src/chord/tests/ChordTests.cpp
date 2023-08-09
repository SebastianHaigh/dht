#include <catch2/catch_test_macros.hpp>

#include <cstdint>
#include <memory>
#include <thread>
#include <bit>

#include "../ChordNode.h"
#include "../ChordMessaging.h"
#include "../NodeId.h"
#include "../../networkSimulation/NetworkSimulation.h"

namespace chord { namespace test {

class MockConnectionManager : public ConnectionManager_I
{
  public:
    MockConnectionManager(const NodeId& nodeId, SimulatedNode& node, std::unique_ptr<logging::Logger> logger)
      : m_nodeId(nodeId),
        m_simulatedNode(node),
        m_logger(std::move(logger)),
        m_logPrefix(m_nodeId.toString() + ": ")
    {
    }

    ~MockConnectionManager()
    {
      stop();
    }

    bool send(const NodeId& nodeId, const Message& message) override
    {
      m_logger->log(m_logPrefix + "sending message to " + nodeId.toString());
      uint32_t ip{ 0 };
      bool foundNode{ false };

      for (const auto& idIpPair : m_nodeIdToIp)
      {
        if (idIpPair.first == nodeId)
        {
          ip = idIpPair.second;
          foundNode = true;
          break;
        }
      }

      if (not foundNode)
      {
        m_logger->log(m_logPrefix + "could not find node " + nodeId.toString() + " failed to send message");
        return false;
      }
      m_logger->log(m_logPrefix + "found ip for " + nodeId.toString() + " sending message to " + std::to_string(ip));

      auto encoded = message.encode();

      m_simulatedNode.sendMessage(ip, encoded.m_message, encoded.m_length);

      return true;
    }

    bool broadcast(const Message& message) override
    {
      m_logger->log(m_logPrefix + "broadcasting message");
      for (const auto& idIpPair : m_nodeIdToIp)
      {
        auto encoded = message.encode();

        m_simulatedNode.sendMessage(idIpPair.second, encoded.m_message, encoded.m_length);
        m_logger->log(m_logPrefix + "message sent to " + idIpPair.first.toString() + "(" + std::to_string(idIpPair.second) + ") as part of broadcast");
      }

      return true;
    }

    void registerReceiveHandler(tcp::OnReceiveCallback callback) override
    {
      m_onReceive = callback;

      NodeReceiveHandler handler = [this] (uint32_t sourceIp, uint8_t* message, std::size_t messageLength)
      {
        if (m_onReceive)
        {
          m_onReceive(message, messageLength);
        }
      };

      m_simulatedNode.registerReceiveHandler(handler);
    }

    void insert(const NodeId& id, uint32_t ipAddress, uint16_t port) override
    {
      for (const auto& idIpPair : m_nodeIdToIp)
      {
        // There is already a connection to this node
        if (idIpPair.first == id) return;
      }

      m_nodeIdToIp.emplace_back(id, ipAddress);
    }

    void remove(const NodeId& id) override
    {
    }

    void stop() override
    {
      m_simulatedNode.cancelReceiveHandler();
      m_onReceive = nullptr;
    }

    [[nodiscard]] uint32_t ip() const override
    {
      return m_simulatedNode.ip();
    }

    [[nodiscard]] uint32_t ip(const NodeId& nodeId) const override
    {
      for (const auto& idIpPair : m_nodeIdToIp)
      {
        if (idIpPair.first == nodeId)
        {
          return idIpPair.second;
        }
      }

      return 0;
    }

  private:
    NodeId m_nodeId;
    SimulatedNode& m_simulatedNode;

    std::vector<std::pair<NodeId, uint32_t>> m_nodeIdToIp;

    tcp::OnReceiveCallback m_onReceive;
    std::unique_ptr<logging::Logger> m_logger;
    const std::string m_logPrefix;
};

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
  NetworkSimulator networkSimulator;
  logging::Log log;

  ConnectionManagerFactory factory = [&networkSimulator, &log] (const NodeId& nodeId, uint32_t ipAddress, uint16_t port)
  {
    return std::make_unique<MockConnectionManager>(nodeId, networkSimulator.addNode(ipAddress), log.makeLogger("CONMAN"));
  };

  ChordNode node0{"node0", "200.178.0.1", 0, factory, log.makeLogger("CHORDNODE")};

  // Create another node and join the network that is currently formed by the first node

  ChordNode node1{"node1", "200.178.0.5", 0, factory, log.makeLogger("CHORDNODE")};

  node0.create();

  node1.join("200.178.0.1");

  std::this_thread::sleep_for(std::chrono::seconds{10});

  std::cout << "node0 succ " << node0.getSuccessorId().toString() << ", pred " << node0.getPredecessorId().toString() << std::endl;
  std::cout << "node1 succ " << node1.getSuccessorId().toString() << ", pred " << node1.getPredecessorId().toString()<< std::endl;
  CHECK(node0.getSuccessorId() == node1.getId());
  CHECK(node0.getPredecessorId() == node1.getId());
  CHECK(node1.getSuccessorId() == node0.getId());
  CHECK(node1.getPredecessorId() == node0.getId());
}

TEST_CASE("Test fixing the fingers")
{
  NetworkSimulator networkSimulator;
  logging::Log log;

  ConnectionManagerFactory factory = [&networkSimulator, &log] (const NodeId& nodeId, uint32_t ipAddress, uint16_t port)
  {
    return std::make_unique<MockConnectionManager>(nodeId, networkSimulator.addNode(ipAddress), log.makeLogger("CONMAN"));
  };

  ChordNode node0{"node0", "200.178.0.1", 0, factory, log.makeLogger("CHORDNODE")};
  ChordNode node1{"node1", "200.178.0.5", 0, factory, log.makeLogger("CHORDNODE")};
  ChordNode node2{"node2", "200.178.0.10", 0, factory, log.makeLogger("CHORDNODE")};
  node0.create();

  node1.join("200.178.0.1");

  std::this_thread::sleep_for(std::chrono::seconds{10});
  std::cout << "node0 " << node0.getId().toString() << " succ " << node0.getSuccessorId().toString() << ", pred " << node0.getPredecessorId().toString() << std::endl;
  std::cout << "node1 " << node1.getId().toString() << " succ " << node1.getSuccessorId().toString() << ", pred " << node1.getPredecessorId().toString()<< std::endl;

  node2.join("200.178.0.5");

  std::this_thread::sleep_for(std::chrono::seconds{10});
  ChordNode node3{"node3", "200.178.0.15", 0, factory, log.makeLogger("CHORDNODE")};
  node3.join("200.178.0.10");
  std::this_thread::sleep_for(std::chrono::seconds{5});

  ChordNode node4{"node4", "200.178.0.20", 0, factory, log.makeLogger("CHORDNODE")};
  node4.join("200.178.0.10");
  std::this_thread::sleep_for(std::chrono::seconds{5});

  ChordNode node5{"node5", "200.178.0.25", 0, factory, log.makeLogger("CHORDNODE")};
  node5.join("200.178.0.15");
  std::this_thread::sleep_for(std::chrono::seconds{40});

  std::cout << "node0 " << node0.getId().toString() << " succ " << node0.getSuccessorId().toString() << ", pred " << node0.getPredecessorId().toString() << std::endl;
  std::cout << "node1 " << node1.getId().toString() << " succ " << node1.getSuccessorId().toString() << ", pred " << node1.getPredecessorId().toString()<< std::endl;
  std::cout << "node2 " << node2.getId().toString() << " succ " << node2.getSuccessorId().toString() << ", pred " << node2.getPredecessorId().toString()<< std::endl;
  std::cout << "node3 " << node3.getId().toString() << " succ " << node3.getSuccessorId().toString() << ", pred " << node3.getPredecessorId().toString()<< std::endl;
  std::cout << "node4 " << node4.getId().toString() << " succ " << node4.getSuccessorId().toString() << ", pred " << node4.getPredecessorId().toString()<< std::endl;
  std::cout << "node5 " << node5.getId().toString() << " succ " << node5.getSuccessorId().toString() << ", pred " << node5.getPredecessorId().toString()<< std::endl;

  std::this_thread::sleep_for(std::chrono::seconds{10});
//  REQUIRE(node0.getSuccessorId() == node1.getPredecessorId());
//  REQUIRE(node1.getPredecessorId() == node0.getSuccessorId());
}

TEST_CASE("Chord messaging test")
{
  NodeId nodeId { "12345678-abcdabcd-effeeffe-dcbadcba-87654321" };
  NodeId sourceNodeId { "87654321-abcdabcd-eff00ffe-dcbadcba-12345678" };

  FindSuccessorMessage message{CommsVersion::V1, nodeId, sourceNodeId, 3987};

  EncodedMessage encoded = message.encode();

  FindSuccessorMessage decodedMessage{CommsVersion::V1};

  decodedMessage.decode(std::move(encoded));

  REQUIRE(decodedMessage.queryNodeId() == nodeId);
  REQUIRE(decodedMessage.sourceNodeId() == sourceNodeId);
  REQUIRE(decodedMessage.requestId() == 3987);
}

}} // namespace chord::test

