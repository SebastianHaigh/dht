#include "../networkSimulation/NetworkSimulation.h"

enum class OpType
{
  LEARN_COMM_GRAPH,
  BUILD_SPANNING_TREE,
};

enum class MessageType : uint16_t
{
  POSITION,
};

struct NodeConfig
{
  int m_id;
  uint32_t m_neighbourIpAddresses[5];
};

struct PositionMessage
{
  PositionMessage() : id(0), m_neighbours{0} {};

  PositionMessage(int id, uint32_t* neighbours, size_t numNeighbours)
    : id(id),
      m_neighbours{0}
  {
    numNeighbours = (numNeighbours > 5) ? 5 : numNeighbours;

    for (int i = 0; i < numNeighbours; i++)
    {
      m_neighbours[i] = neighbours[i];
    }
  }

  PositionMessage(int id, std::vector<uint32_t> neighbours)
    : id(id),
      m_neighbours{0}
  {
    for (int i = 0; i < 5; i++)
    {
      m_neighbours[i] = neighbours[i];
    }
  }
  int id;
  uint32_t m_neighbours[5];
};

class Node
{
  public:
    Node(const SimulatedNode& node, NodeConfig config);

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
