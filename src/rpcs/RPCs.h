#include "../networkSimulation/NetworkSimulation.h"

enum class OpType
{
  LEARN_COMM_GRAPH,
  BUILD_SPANNING_TREE,
};

class Node
{
  public:
    Node(const SimulatedNode& node) : m_node(node) {};

    void start(const OpType& opType);

  private:
    void go();
    void back();
    void position(int id, std::vector<int>& neighbours);

    const SimulatedNode& m_node;
    int m_id;
    std::vector<int> m_nodesKnown;
    std::vector<std::pair<int, int>> m_channelsKnown;
};
