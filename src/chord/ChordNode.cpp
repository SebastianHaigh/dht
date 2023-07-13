#include "ChordNode.h"
#include "ChordMessaging.h"
#include "../comms/CommsCoder.h"

#include <algorithm>
#include <arpa/inet.h>

namespace chord {

ChordNode::ChordNode(const std::string& ip,
                     uint16_t port,
                     const ConnectionManagerFactory& connectionManagerFactory)
  : m_id(createNodeId(ip)),
    m_predecessor{},
    m_successor{},
    m_fingerTable(m_id),
    m_ipAddress{convertIpAddressToInteger(ip)},
    m_port{port},
    m_connectionManager(connectionManagerFactory(NodeId{m_ipAddress}, m_ipAddress, port))
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
  m_connectionManager->insert(knownNodeId, ip, m_port);

  // Send a request to the known node to find the successor of this node
  auto requestId = getNextAvailableRequestId();

  PendingMessageResponse pending;
  pending.m_type = MessageType::CHORD_FIND_SUCCESSOR_RESPONSE;
  pending.m_nodeId = knownNodeId;
  pending.m_hasChain = false;

  m_pendingResponses.emplace(requestId, pending);

  std::promise<NodeId> findSuccessorPromise;
  std::future<NodeId> findSuccessorFuture = findSuccessorPromise.get_future();

  m_findSuccessorPromises.emplace(requestId, std::move(findSuccessorPromise));

  FindSuccessorMessage message{ CommsVersion::V1, m_id, m_id, requestId };

  m_connectionManager->send(knownNodeId, message);

  // We are going to wait here until we get a response
  m_successor = findSuccessorFuture.get();
}

const NodeId &ChordNode::getPredecessorId()
{
  return m_predecessor;
}

const NodeId &ChordNode::getSuccessorId()
{
  return m_successor;
}

void ChordNode::doFindSuccessor(const FindSuccessorMessage& message)
{
  if (message.queryNodeId() < m_id && message.queryNodeId() > m_predecessor)
  {
    auto nodeId = closestPrecedingFinger(message.queryNodeId());

    if (nodeId == m_id)
    {
      // If this is the case then we can start returning 
      FindSuccessorResponseMessage response{ CommsVersion::V1, m_id, m_id, message.requestId() };

      m_connectionManager->send(message.sourceNodeId(), response);
    }

    auto requestId = getNextAvailableRequestId();

    PendingMessageResponse pending;
    pending.m_type = MessageType::CHORD_FIND_SUCCESSOR_RESPONSE;
    pending.m_nodeId = nodeId;
    pending.m_hasChain = true;
    pending.m_chainingDestination = message.sourceNodeId();

    m_pendingResponses.emplace(requestId, pending);

    FindSuccessorMessage messageToForward{ CommsVersion::V1, message.queryNodeId(), m_id, requestId };

    m_connectionManager->send(nodeId, messageToForward);
  }

}

void ChordNode::handleFindSuccessorResponse(const FindSuccessorResponseMessage& message)
{
  auto it = m_pendingResponses.find(message.requestId());

  if (it == m_pendingResponses.end())
  {
    std::cout << "Unexpected find successor response from node: " << message.sourceNodeId().toString() << std::endl;
    return;
  }

  if (it->second.m_hasChain)
  {
    // This message is not for us, forward it on
    FindSuccessorResponseMessage messageToForward{ CommsVersion::V1,
                                                   message.nodeId(),
                                                   m_id,
                                                   message.requestId() };

    m_connectionManager->send(it->second.m_chainingDestination, messageToForward);

    m_pendingResponses.erase(it);

    return;
  }

  // This message is for us, there should be a promise waiting for the result
  auto promiseIter = m_findSuccessorPromises.find(message.requestId());

  if (promiseIter == m_findSuccessorPromises.end())
  {
    std::cout << "Expected promise, but there wasn't one" << std::endl;
    return;
  }

  promiseIter->second.set_value(message.nodeId());

  m_findSuccessorPromises.erase(promiseIter);
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

void ChordNode::updateFingerTable(const ChordNode &node, uint16_t i)
{
  
}

void ChordNode::handleReceivedMessage(const EncodedMessage& encoded)
{
  // Ignore comms version for now, this will probably be handled differently at a later time

  // get the message type, which is a uint32_t starting at encoded.message[2]
  uint32_t type;
  decodeSingleValue(&encoded.m_message[2], &type);

  switch (static_cast<MessageType>(type))
  {
    case MessageType::CHORD_FIND_SUCCESSOR:
    {
      FindSuccessorMessage message{ CommsVersion::V1 };

      message.decode(std::move(encoded));

      doFindSuccessor(message);
      break;
    }

    case MessageType::CHORD_FIND_SUCCESSOR_RESPONSE:
    {
      FindSuccessorResponseMessage message{ CommsVersion::V1 };

      message.decode(std::move(encoded));

      handleFindSuccessorResponse(message);
      break;
    }

    default:
    {
      // This should probably be logged
    }
  }
}

uint32_t ChordNode::getNextAvailableRequestId()
{
  // Zero is a null value for requestId, so always skip it
  if (m_requestIdCounter == 0) m_requestIdCounter++;

  auto it = m_pendingResponses.find(m_requestIdCounter);

  if (it == m_pendingResponses.end()) return m_requestIdCounter++;

  while (it != m_pendingResponses.end())
  {
    it = m_pendingResponses.find(m_requestIdCounter);
    if (it == m_pendingResponses.end()) return m_requestIdCounter++;
  }

  // should never get here, but this return statement suppresses a compiler warning
  return 0;
}

} // namespace chord

