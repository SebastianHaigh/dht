#include "../networkSimulation/NetworkSimulation.h"
#include "../async/ThreadPool.h"
#include "../comms/Comms.h"

#include <cstdint>

enum class OpType
{
  LEARN_COMM_GRAPH,
  BUILD_SPANNING_TREE,
};

struct NodeConfig
{
  int m_id;
  uint32_t m_ip;
  uint32_t m_neighbourIpAddresses[5];
};

class Node
{
  public:
    Node(SimulatedNode& node, NodeConfig config);

    void run();

    void start(const OpType& opType);

    void receiveMessage(uint32_t sourceIpAddress, EncodedMessage encodedMessage);

  private:
    void position(const PositionMessage& positionMessage);

    void handleJoinRequest(const JoinMessage& message);
    void handleJoinResponse(const JoinResponseMessage& message);

    SimulatedNode& m_node;

    ThreadPool m_workQueue;

    int m_id;
    uint32_t m_ip;

    std::vector<uint32_t> m_neighbourIps;
    std::vector<int> m_neighbourIds;

    // For learning the comms graph
    std::vector<int> m_nodesKnown;
    std::vector<std::pair<int, int>> m_channelsKnown;
    bool m_part = false;

};
