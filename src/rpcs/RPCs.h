#include "../networkSimulation/NetworkSimulation.h"
#include "Messages.h"

enum class OpType
{
  LEARN_COMM_GRAPH,
  BUILD_SPANNING_TREE,
};

struct NodeConfig
{
  int m_id;
  uint32_t m_neighbourIpAddresses[5];
};

class Node
{
  public:
    Node(SimulatedNode& node, NodeConfig config);

    void start(const OpType& opType);

  private:
    void go();
    void back();
    void position(int id, std::vector<int>& neighbours);

    const SimulatedNode& m_node;

    int m_id;
    std::vector<uint32_t> m_neighbourIps;
    std::vector<int> m_neighbourIds;

    std::vector<int> m_nodesKnown;
    std::vector<std::pair<int, int>> m_channelsKnown;
};
