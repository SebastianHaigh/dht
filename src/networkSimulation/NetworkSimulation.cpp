#include "NetworkSimulation.h"

#include <cstdint>
#include <memory>
#include <utility>

NetworkSimulator::NetworkSimulator()
  : m_nextNodeId(0)
{
}

void NetworkSimulator::run()
{
}

const SimulatedNode& NetworkSimulator::addNode(uint32_t ipAddress)
{
  m_nodeIds.push_back(m_nextNodeId++);

  auto onSendCallback = [this] (uint32_t source, uint32_t dest, int message) { sendMessage(source, dest, message); };

  auto node = std::make_unique<SimulatedNode>(m_nodeIds.back(),
                                              ipAddress,
                                              onSendCallback);

  m_nodes.emplace_back(m_nodeIds.back(), ipAddress);

  if (m_nodes.size() == 1) return *m_nodes.back();

  for (const auto& node : m_nodes)
  {
    if (node->nodeId() != m_nodeIds.back())
    {
      m_links.emplace_back(node.get(), m_nodes.back().get());
    }
  }

  return *m_nodes.back();
}

void sendMessage(uint32_t sourceIpAddress, uint32_t destinationIpAddress, int message)
{
}

void NetworkSimulator::removeAllLinksForNode(int nodeId)
{
  auto it = m_links.begin();

  while (it != m_links.end())
  {
    if (it->hasNodeId(nodeId))
    {
      it = m_links.erase(it);
      continue;
    }
    it++;
  }
}

SimulatedNode::SimulatedNode(int nodeId, int ipAddress, OnSendCallback onSendCallback)
  : m_nodeId(nodeId),
    m_ipAddress(ipAddress),
    m_onSendCallback(std::move(onSendCallback))
{
}

void SimulatedNode::run()
{
}

void SimulatedNode::sendMessage(uint32_t sourceIpAddress, uint32_t destinationIpAddress, int message)
{
  m_onSendCallback(sourceIpAddress, destinationIpAddress, message);
}

int SimulatedNode::nodeId() const
{
  return m_nodeId;
}

SimulatedLink::SimulatedLink(const SimulatedNode* node1, const SimulatedNode* node2)
  : m_node1(node1),
    m_node2(node2),
    m_up(false)
{
}

void SimulatedLink::run()
{
}

bool SimulatedLink::hasNodeId(int nodeId) const
{
  if (m_node1 == nullptr || m_node2 == nullptr)
  {
    return false;
  }

  return (m_node1->nodeId() == nodeId ||
          m_node2->nodeId() == nodeId);
}

