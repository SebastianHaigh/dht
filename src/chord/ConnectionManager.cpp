#include "ConnectionManager.h"

namespace odd::chord {

ConnectionManager::ConnectionManager(const NodeId& nodeId, uint32_t ip, uint16_t port)
  : m_server{ip, port},
    m_localNodeId(nodeId),
    m_localIpAddress(ip),
    m_localPort(port)
  {
  }

bool ConnectionManager::send(const NodeId& nodeId, const Message& message)
{
  auto nodeConnection = getNodeConnection(nodeId);

  if (nodeConnection == m_nodeConnections.end()) return false;
  if (nodeConnection->m_id != nodeId) return false;

  auto encoded = message.encode();

  nodeConnection->m_tcpClient->send(encoded.m_message, encoded.m_length);

  return true;
}

void ConnectionManager::registerReceiveHandler(tcp::OnReceiveCallback callback)
{
  m_server.subscribeToAll(std::move(callback));
}

void ConnectionManager::insert(const NodeId& id, uint32_t ipAddress, uint16_t port)
{
  auto nodeConnection = getNodeConnection(id);

  if (nodeConnection == m_nodeConnections.end())
  {
    m_nodeConnections.emplace_back(id, ipAddress, port);
    return;
  }

  m_nodeConnections.insert(nodeConnection, NodeConnection{id, ipAddress, port});
}

void ConnectionManager::remove(const NodeId& id)
{
  if (m_nodeConnections.empty()) return;

  auto nodeConnection = getNodeConnection(id);

  if (nodeConnection == m_nodeConnections.end()) return;
  if (nodeConnection->m_id != id) return;

  m_nodeConnections.erase(nodeConnection);
}

std::vector<ConnectionManager::NodeConnection>::iterator 
ConnectionManager::getNodeConnection(const NodeId& nodeId)
{
  return std::lower_bound(m_nodeConnections.begin(),
                          m_nodeConnections.end(),
                          nodeId,
                          [](const NodeConnection& lhs, const NodeId& rhs)
                          {
                            return lhs.m_id < rhs;
                          });
}

std::vector<ConnectionManager::NodeConnection>::const_iterator
ConnectionManager::getNodeConnection(const NodeId& nodeId) const
{
  return const_cast<ConnectionManager*>(this)->getNodeConnection(nodeId);
}

std::size_t ConnectionManager::getClientIndex(const NodeId& nodeId)
{
  auto nodeConnection = getNodeConnection(nodeId);

  return std::distance(m_nodeConnections.begin(), nodeConnection);
}

[[nodiscard]] uint32_t ConnectionManager::ip() const
{
  return m_localIpAddress;
}

[[nodiscard]] uint32_t ConnectionManager::ip(const NodeId& nodeId) const
{
  auto it = getNodeConnection(nodeId);

  if (it == m_nodeConnections.end()) return 0;

  return 0;
}

} // namespace odd::chord
 
