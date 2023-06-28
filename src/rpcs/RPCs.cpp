#include "RPCs.h"


Node::Node(SimulatedNode& node, NodeConfig config)
  : m_node(node),
    m_id(config.m_id)
{
  for (unsigned int neighbourIpAddress : config.m_neighbourIpAddresses)
  {
    if (neighbourIpAddress == 0) return;

    m_neighbourIps.push_back(neighbourIpAddress);
  }
};

void Node::start(const OpType& opType)
{
}

void go()
{
}
