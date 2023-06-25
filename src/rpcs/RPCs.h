#include "../networkSimulation/NetworkSimulation.h"

enum class OpType
{
  BUILD_SPANNING_TREE,
};

class Node
{
  public:
    Node(const SimulatedNode& node) : m_node(node) {};

    void start(const OpType& opType);

    void go();
    void back();

  private:
    const SimulatedNode& m_node;
};
