#include "ChordNode.h"
#include "ChordMessaging.h"

#include <algorithm>
#include <arpa/inet.h>

namespace chord {

ChordNode::ChordNode(const std::string& ip, uint16_t port)
  : m_id(createNodeId(ip)),
    m_predecessor{},
    m_successor{},
    m_fingerTable(m_id),
    m_ipAddress{convertIpAddressToInteger(ip)},
    m_port{port},
    m_connectionManager(NodeId{m_ipAddress}, m_ipAddress, port)
{
}

NodeId ChordNode::createNodeId(const std::string& ipAddress)
{
  hashing::SHA1Hash digest;
  hashing::sha1((uint8_t*) ipAddress.c_str(), ipAddress.length(), digest);

  return NodeId{ digest };
}

uint32_t ChordNode::convertIpAddressToInteger(const std::string& ipAddress)
{
  struct sockaddr_in sa{};
  inet_pton(AF_INET, ipAddress.c_str(), &(sa.sin_addr));
  return sa.sin_addr.s_addr;
}

void ChordNode::join(const std::string &knownNodeIpAddress)
{
  // The first thing we need to do is create a TcpClient in order to establish a connection to this node
  auto ip = convertIpAddressToInteger(knownNodeIpAddress);

  // Create a NodeId from the ip address
  NodeId knownNodeId{ ip };

  // Insert the node connection into the node connections vector in sorted order
  m_connectionManager.insert(knownNodeId, ip, m_port);

  // Send a request to the known node to find the successor of this node
  FindSuccessorMessage message {CommsVersion::V1, m_id};

  m_connectionManager.send(knownNodeId, message);
}

const NodeId &ChordNode::getPredecessorId()
{
  return m_predecessor;
}

const NodeId &ChordNode::getSuccessorId()
{
  return m_successor;
}

std::future<NodeId> ChordNode::findSuccessor(const NodeId &id)
{
  std::function<NodeId()> taskFn = [this, id] () -> NodeId { return doFindSuccessor(id); }; 
  return m_threadPool.post(taskFn);
}

NodeId ChordNode::doFindSuccessor(const NodeId &id)
{
  if (id < m_id && id > m_predecessor)
  {
    auto nodeId = closestPrecedingFinger(id);
    
    if (nodeId == m_id)
    {
      return m_successor;
    }

    FindSuccessorMessage message { CommsVersion::V1, id };

    m_connectionManager.send(nodeId, message);
  }

  return m_successor;

}

const NodeId &ChordNode::closestPrecedingFinger(const NodeId &id)
{
  for (int i = 159; i >= 0; i--)
  {
    if (m_fingerTable[i].m_nodeId > m_id && m_fingerTable[i].m_nodeId < id)
    {
      return m_fingerTable[i].m_nodeId;
    }
  }

  return m_id;
}

void ChordNode::initialiseFingerTable()
{
  for (int i = 0; i < 160; i++)
  {
  }
}

void ChordNode::updateOthers()
{

}

void ChordNode::updateFingerTable(const ChordNode &node, uint16_t i)
{
  
}

void ChordNode::processReceivedMessage(const Message& request)
{
  switch (request.type())
  {
    case MessageType::CHORD_FIND_SUCCESSOR:
    {
      break;
    }
    default:
    {
      // Not a chord message
      break;
    }
  }
}

} // namespace chord

