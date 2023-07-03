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
    m_tcpServer{ip, port}
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
  hashing::SHA1Hash digest;
  hashing::sha1((uint8_t*) knownNodeIpAddress.c_str(), knownNodeIpAddress.length(), digest);

  // Create a NodeId from the digest
  NodeId knownNodeId{ digest };

  // Create a TcpClient to connect to the known node
  NodeConnection nodeConnection{knownNodeId, knownNodeIpAddress, 5000};

  // Insert the node connection into the node connections vector in sorted order
  auto it = std::lower_bound(m_nodeConnections.begin(), 
                             m_nodeConnections.end(), 
                             nodeConnection, 
                             [] (const NodeConnection& lhs, const NodeConnection& rhs) 
                             { 
                               return lhs.m_id < rhs.m_id;
                             });

  m_nodeConnections.insert(it, std::move(nodeConnection));

  // Send a request to the known node to find the successor of this node
  sendFindSuccessorRequest(m_id);
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

    sendFindPredecessorRequest(nodeId);
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

void ChordNode::sendFindSuccessorRequest(const NodeId &id)
{
  auto nodeConnection = std::lower_bound(m_nodeConnections.begin(), 
                                         m_nodeConnections.end(), 
                                         id,
                                         [](const NodeConnection& lhs, const NodeId& rhs) 
                                         { 
                                           return lhs.m_id < rhs;
                                         });

  if (nodeConnection != m_nodeConnections.end())
  {
    FindSuccessorMessage message{ CommsVersion::V1, id };

    auto encoded = message.encode();

    nodeConnection->m_tcpClient->send(encoded.m_message, encoded.m_length);
  }
}

void ChordNode::sendFindPredecessorRequest(const NodeId &id)
{
  auto nodeConnection = std::lower_bound(m_nodeConnections.begin(), 
                                         m_nodeConnections.end(), 
                                         id,
                                         [](const NodeConnection& lhs, const NodeId& rhs) 
                                         { 
                                           return lhs.m_id < rhs;
                                         });

  if (nodeConnection != m_nodeConnections.end())
  {
    FindSuccessorMessage message{ CommsVersion::V1, id };

    auto encoded = message.encode();

    nodeConnection->m_tcpClient->send(encoded.m_message, encoded.m_length);
  }
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

