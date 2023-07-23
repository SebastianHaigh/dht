#include "ChordNode.h"
#include "ChordMessaging.h"
#include "../comms/CommsCoder.h"

#include <algorithm>
#include <arpa/inet.h>

namespace chord {

ChordNode::ChordNode(const std::string& nodeName,
                     const std::string& ip,
                     uint16_t port,
                     const ConnectionManagerFactory& connectionManagerFactory)
  : m_nodeName(nodeName),
    m_ipAddress{convertIpAddressToInteger(ip)},
    m_id(m_ipAddress),
    m_predecessor{std::nullopt},
    m_successor{m_id},
    m_port{port},
    m_connectionManager(connectionManagerFactory(NodeId{m_ipAddress}, m_ipAddress, port))
{
  ::chord::initialiseFingerTable(m_fingerTable, m_id);

  tcp::OnReceiveCallback onReceiveCallback = [this] (uint8_t* message, std::size_t messageLength)
  {
    receive(message, messageLength);
  };

  m_connectionManager->registerReceiveHandler(onReceiveCallback);

}

uint32_t ChordNode::convertIpAddressToInteger(const std::string& ipAddress)
{
  struct sockaddr_in sa{};
  inet_pton(AF_INET, ipAddress.c_str(), &(sa.sin_addr));
  return sa.sin_addr.s_addr;
}

void ChordNode::create()
{
  m_predecessor = std::nullopt;
  m_successor = m_id;
}

void ChordNode::join(const std::string &knownNodeIpAddress)
{
  m_predecessor = std::nullopt;

  // The first thing we need to do is create a TcpClient in order to establish a connection to this node
  auto ip = convertIpAddressToInteger(knownNodeIpAddress);

  // Create a NodeId from the ip address
  NodeId knownNodeId{ ip };

  // Insert the node connection into the node connections vector in sorted order
  m_connectionManager->insert(knownNodeId, ip, m_port);

  // Send a request to the known node to find the successor of this node
  auto requestId = getNextAvailableRequestId();

  PendingMessageResponse pending;
  pending.m_type = MessageType::JOIN_RESPONSE;
  pending.m_nodeId = knownNodeId;
  pending.m_hasChain = false;

  m_pendingResponses.emplace(requestId, pending);

  std::future<NodeId> joinFuture = m_joinPromise.get_future();

  JoinMessage joinMessage{ CommsVersion::V1, m_connectionManager->ip(), requestId };

  log("sending JoinMessage");
  m_connectionManager->send(knownNodeId, joinMessage);

  joinFuture.wait();

  std::future<NodeId> findSuccessorFuture = findSuccessor(knownNodeId, m_id);

  // We are going to wait here until we get a response
  m_successor = findSuccessorFuture.get();

  log("successor found, set to : " + m_successor.toString());

  // After this we need to initialise the finger table by calling stabilise
  stabilise();
}

const std::optional<NodeId> &ChordNode::getPredecessorId()
{
  return m_predecessor;
}

const NodeId &ChordNode::getSuccessorId()
{
  return m_successor;
}

void ChordNode::receive(uint8_t* message, std::size_t messageLength)
{
  log("receiving message");
  EncodedMessage encoded{message, messageLength};

  handleReceivedMessage(encoded);
}

void ChordNode::doFindSuccessor(const FindSuccessorMessage& message)
{
  log("finding successor for " + message.queryNodeId().toString());
  std::cout << "    m_id " << m_id.toString() << std::endl;
  std::cout << "    m_successor " << m_successor.toString() << std::endl;
  std::cout << "    query " << message.queryNodeId().toString() << std::endl;

  if (containedInOpenInterval(m_id, m_successor, message.queryNodeId()))
  {
    log("successor found for " + message.queryNodeId().toString());

    // If this is the case then we can start returning
    FindSuccessorResponseMessage response{ CommsVersion::V1, m_successor, m_id, message.requestId() };

    log("sending FindSuccessorResponse");
    m_connectionManager->send(message.sourceNodeId(), response);

    return;
  }

  auto nodeId = closestPrecedingFinger(message.queryNodeId());

  log("could not find successor for " + message.queryNodeId().toString());
  log("forwarding message to " + nodeId.toString());

  auto requestId = getNextAvailableRequestId();

  PendingMessageResponse pending;
  pending.m_type = MessageType::CHORD_FIND_SUCCESSOR_RESPONSE;
  pending.m_nodeId = nodeId;
  pending.m_hasChain = true;
  pending.m_chainingDestination = message.sourceNodeId();

  m_pendingResponses.emplace(requestId, pending);

  FindSuccessorMessage messageToForward{ CommsVersion::V1, message.queryNodeId(), m_id, requestId };

  log("sending FindSuccessorMessage");
  m_connectionManager->send(nodeId, messageToForward);
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

    log("sending FindSuccessorResponse");
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

std::future<NodeId> ChordNode::findSuccessor(const NodeId& hash)
{
  return findSuccessor(closestPrecedingFinger(hash), hash);
}

std::future<NodeId> ChordNode::findSuccessor(const NodeId& nodeToQuery, const NodeId& hash)
{
  auto requestId = getNextAvailableRequestId();

  PendingMessageResponse pending;
  pending.m_type = MessageType::CHORD_FIND_SUCCESSOR_RESPONSE;
  pending.m_nodeId = hash;
  pending.m_hasChain = false;

  m_pendingResponses.emplace(requestId, pending);

  std::promise<NodeId> findSuccessorPromise;
  std::future<NodeId> findSuccessorFuture = findSuccessorPromise.get_future();

  m_findSuccessorPromises.emplace(requestId, std::move(findSuccessorPromise));

  FindSuccessorMessage message{ CommsVersion::V1, hash, m_id, requestId };

  log("sending FindSuccessorMessage");
  m_connectionManager->send(nodeToQuery, message);

  return findSuccessorFuture;
}

std::future<ChordNode::Neighbours> ChordNode::getNeighbours(const NodeId& nodeToQuery)
{
  auto requestId = getNextAvailableRequestId();

  PendingMessageResponse pending;
  pending.m_type = MessageType::CHORD_GET_NEIGHBOURS_RESPONSE;
  pending.m_nodeId = nodeToQuery;
  pending.m_hasChain = false;

  m_pendingResponses.emplace(requestId, pending);

  std::promise<Neighbours> getNeighboursPromise;
  std::future<Neighbours> getNeighboursFuture = getNeighboursPromise.get_future();

  m_getNeighboursPromises.emplace(requestId, std::move(getNeighboursPromise));

  GetNeighboursMessage message{ CommsVersion::V1, m_id, requestId };

  log("sending GetNeighboursMessage");
  m_connectionManager->send(nodeToQuery, message);

  return getNeighboursFuture;
}

void ChordNode::notify(const NodeId& nodeId)
{
  NotifyMessage message{ CommsVersion::V1, m_id };

  log("sending NotifyMessage");
  m_connectionManager->send(nodeId, message);
}

const NodeId &ChordNode::closestPrecedingFinger(const NodeId &id)
{
  for (int i = 159; i >= 0; i--)
  {
    if (containedInOpenInterval(m_id, id, m_fingerTable.m_fingers[i].m_nodeId))
    {
      return m_fingerTable.m_fingers[i].m_nodeId;
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
    case MessageType::JOIN:
    {
      log("received join request");
      JoinMessage message{ CommsVersion::V1 };

      message.decode(std::move(encoded));

      handleJoinRequest(message);
      break;
    }
    case MessageType::JOIN_RESPONSE:
    {
      log("received join response");
      JoinResponseMessage message{ CommsVersion::V1 };

      message.decode(std::move(encoded));

      handleJoinResponse(message);
      break;
    }
    case MessageType::CHORD_FIND_SUCCESSOR:
    {
      log("received chord find successor request");
      FindSuccessorMessage message{ CommsVersion::V1 };

      message.decode(std::move(encoded));

      doFindSuccessor(message);
      break;
    }

    case MessageType::CHORD_FIND_SUCCESSOR_RESPONSE:
    {
      log("received chord find successor response");
      FindSuccessorResponseMessage message{ CommsVersion::V1 };

      message.decode(std::move(encoded));

      handleFindSuccessorResponse(message);
      break;
    }

    case MessageType::CHORD_NOTIFY:
    {
      log("received chord notify message");
      NotifyMessage message{ CommsVersion::V1 };

      message.decode(std::move(encoded));

      handleNotify(message);
      break;
    }

    case MessageType::CHORD_GET_NEIGHBOURS:
    {
      log("received chord get neighbours request") ;
      GetNeighboursMessage message{ CommsVersion::V1 };

      message.decode(std::move(encoded));

      handleGetNeighbours(message);
      break;
    }

    case MessageType::CHORD_GET_NEIGHBOURS_RESPONSE:
    {
      log("received chord get neighbours response") ;
      GetNeighboursResponseMessage message{ CommsVersion::V1 };

      message.decode(std::move(encoded));

      handleGetNeighboursResponse(message);
      break;
    }

    default:
    {
      log("received unknown message type: 0x");
      for (int i = 0; i < encoded.m_length - 1; i++)
      {
        std::cout << std::hex << (uint32_t) encoded.m_message[i] << ", ";
      }

      std::cout << std::hex << encoded.m_message[encoded.m_length - 1] << std::endl;
      // This should probably be logged
    }
  }
}

void ChordNode::handleJoinRequest(const JoinMessage& message)
{
  // This should probably do some checks, but I'm not sure yet what is needed.
  NodeId newNodeId{ message.ip() };

  m_connectionManager->insert(newNodeId, message.ip(), 0);

  JoinResponseMessage joinMessage{ CommsVersion::V1, m_connectionManager->ip(), message.requestId() };

  log("sending JoinResponse");
  m_connectionManager->send(newNodeId, joinMessage);
}

void ChordNode::handleJoinResponse(const JoinResponseMessage& message)
{
  m_joinPromise.set_value(NodeId{ message.ip() });
}

void ChordNode::handleNotify(const NotifyMessage& message)
{
  if (not m_predecessor.has_value() ||
      (containedInOpenInterval(m_predecessor.value(), m_id, message.nodeId())))
  {
    log("setting predecessor to: " + message.nodeId().toString());

    *m_predecessor = message.nodeId();
  }
}

void ChordNode::handleGetNeighbours(const GetNeighboursMessage& message)
{
  GetNeighboursResponseMessage response{ CommsVersion::V1 };

  if (m_predecessor.has_value())
  {
    response = GetNeighboursResponseMessage{ CommsVersion::V1, m_successor, m_predecessor.value(), m_id, message.requestId() };
  }
  else
  {
    response = GetNeighboursResponseMessage{ CommsVersion::V1, m_successor, m_id, message.requestId() };
  }

  log("sending GetNeigboursResponse");
  m_connectionManager->send(message.sourceNodeId(), response);
}

void ChordNode::handleGetNeighboursResponse(const GetNeighboursResponseMessage& message)
{
  auto it = m_pendingResponses.find(message.requestId());

  if (it == m_pendingResponses.end())
  {
    std::cout << "Unexpected get neighbours response from node: " << message.sourceNodeId().toString() << std::endl;
    return;
  }

  // This message is for us, there should be a promise waiting for the result
  auto promiseIter = m_getNeighboursPromises.find(message.requestId());

  if (promiseIter == m_getNeighboursPromises.end())
  {
    std::cout << "Expected promise, but there wasn't one" << std::endl;
    return;
  }

  Neighbours neighbours;
  neighbours.predecessor = message.predecessor();
  neighbours.successor = message.successor();
  neighbours.hasPredecessor = message.hasPredecessor();
  promiseIter->second.set_value(neighbours);

  m_getNeighboursPromises.erase(promiseIter);
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

void ChordNode::stabilise()
{
  log("stabilise");
  std::future<Neighbours> successorNeighboursFuture = getNeighbours(m_successor);

  Neighbours successorNeighbours = successorNeighboursFuture.get();

  if (successorNeighbours.hasPredecessor &&
      successorNeighbours.predecessor > m_id &&
      successorNeighbours.predecessor < m_successor)
  {
    m_successor = successorNeighbours.predecessor;
  }

  notify(m_successor);
}

void ChordNode::log(const std::string& message)
{
  std::cout << "[" << m_nodeName << "-" << m_id.toString() << "] ChordNode: " << message << std::endl;
}

} // namespace chord

