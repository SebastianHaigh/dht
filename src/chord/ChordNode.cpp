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
    m_predecessor{},
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

  m_workThread = std::thread{&ChordNode::workThread, this};
}

uint32_t ChordNode::convertIpAddressToInteger(const std::string& ipAddress)
{
  struct sockaddr_in sa{};
  inet_pton(AF_INET, ipAddress.c_str(), &(sa.sin_addr));
  return sa.sin_addr.s_addr;
}

void ChordNode::create()
{
  m_predecessor = NodeId{};
  m_hasPredecessor = false;
  m_successor = m_id;
}

void ChordNode::join(const std::string &knownNodeIpAddress)
{
  std::function<bool()> joinTask = [this, knownNodeIpAddress] ()
  {
    log("Running join task");
    m_predecessor = NodeId{};
    m_hasPredecessor = false;

    auto ip = convertIpAddressToInteger(knownNodeIpAddress);
    NodeId knownNodeId{ ip };
    m_connectionManager->insert(knownNodeId, ip, m_port);
    m_successor = knownNodeId;

    auto requestId = getNextAvailableRequestId();

    PendingMessageResponse pending;
    pending.m_type = MessageType::JOIN_RESPONSE;
    pending.m_nodeId = knownNodeId;
    pending.m_hasChain = false;

    m_pendingResponses.emplace(requestId, pending);

    m_joinFuture = m_joinPromise.get_future();

    JoinMessage joinMessage{ CommsVersion::V1, m_connectionManager->ip(), requestId };

    log("JoinTask: sending JoinMessage");

    m_connectionManager->send(knownNodeId, joinMessage);

    return true;
  };

  m_queue.putWork(joinTask);

  auto ip = convertIpAddressToInteger(knownNodeIpAddress);
  NodeId knownNodeId{ ip };

  std::function<bool()> findSuccessorTask = [this, knownNodeId] () -> bool
  {
    auto requestId = findSuccessor(knownNodeId, m_id);

    std::function<bool()> checkFutureTask = [this, requestId] () -> bool
    {
      auto it = m_findSuccessorFutures.find(requestId);

      // if the future can't be found then do not load this task again.
      if (it == m_findSuccessorFutures.end()) return true;

      auto futureStatus = it->second.wait_for(std::chrono::milliseconds{20});

      // future not ready, run the task again
      if (futureStatus != std::future_status::ready) return false;

      m_successor = it->second.get();

      log("first findSuccessor has found successor, " + m_successor.toString());

      // successor found, no need to run again
      return true;
    };

    m_queue.putWork(checkFutureTask);

    return true;
  };

  m_queue.putWork(findSuccessorTask);
}

const NodeId& ChordNode::getId() const
{
  return m_id;
}

const NodeId &ChordNode::getPredecessorId() const
{
  return m_predecessor;
}

const NodeId &ChordNode::getSuccessorId() const
{
  return m_successor;
}

void ChordNode::receive(uint8_t* message, std::size_t messageLength)
{
  log("receiving message");
  EncodedMessage encoded{message, messageLength};

  handleReceivedMessage(encoded);
}

void ChordNode::workThread()
{
  while (true)
  {
    m_queue.doNextWork();
    if (std::chrono::high_resolution_clock::now() - m_lastManageTime > std::chrono::seconds{1})
    {
      stabilise();
      fixFingers();
      m_lastManageTime = std::chrono::high_resolution_clock::now();
    }

  }
}

void ChordNode::doFindSuccessor(const FindSuccessorMessage& message)
{
  log("finding successor for " + message.queryNodeId().toString());

  if (containedInLeftOpenInterval(m_id, m_successor, message.queryNodeId()))
  {
    log("successor found for " + message.queryNodeId().toString());

    // If this is the case then we can start returning
    FindSuccessorResponseMessage response{ CommsVersion::V1, m_successor, m_id, m_connectionManager->ip(m_successor), message.requestId() };

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
    log("Unexpected find successor response with ID: " + std::to_string(message.requestId()) + " from node: " + message.sourceNodeId().toString());
    return;
  }

  if (it->second.m_hasChain)
  {
    // This message is not for us, forward it on
    FindSuccessorResponseMessage messageToForward{ CommsVersion::V1,
                                                   message.nodeId(),
                                                   m_id,
                                                   message.ip(),
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

  m_successor = message.nodeId();

  if (message.nodeId() != m_id)
  {
    m_connectionManager->insert(message.nodeId(), message.ip(), 0);
    sendConnect(message.nodeId());
  }

  log("m_successor has been set to " + m_successor.toString());
  m_findSuccessorPromises.erase(promiseIter);
}

uint32_t ChordNode::findSuccessor(const NodeId& hash)
{
  return findSuccessor(closestPrecedingFinger(hash), hash);
}

uint32_t ChordNode::findSuccessor(const NodeId& nodeToQuery, const NodeId& hash)
{
  auto requestId = getNextAvailableRequestId();

  std::promise<NodeId> findSuccessorPromise;
  std::future<NodeId> findSuccessorFuture = findSuccessorPromise.get_future();

  m_findSuccessorFutures.emplace(requestId, std::move(findSuccessorFuture));

  // if the node to query is the current node ID then there is no need to do the RPC
  if (nodeToQuery == m_id)
  {
    findSuccessorPromise.set_value(m_successor);
    return requestId;
  }

  m_findSuccessorPromises.emplace(requestId, std::move(findSuccessorPromise));

  PendingMessageResponse pending;
  pending.m_type = MessageType::CHORD_FIND_SUCCESSOR_RESPONSE;
  pending.m_nodeId = hash;
  pending.m_hasChain = false;

  m_pendingResponses.emplace(requestId, pending);

  FindSuccessorMessage message{ CommsVersion::V1, hash, m_id, requestId };

  log("sending FindSuccessorMessage");
  m_connectionManager->send(nodeToQuery, message);

  return requestId;
}

uint32_t ChordNode::getNeighbours(const NodeId& nodeToQuery)
{
  auto requestId = getNextAvailableRequestId();

  log("get neighbours, request ID " + std::to_string(requestId));

  std::promise<Neighbours> getNeighboursPromise;
  std::future<Neighbours> getNeighboursFuture = getNeighboursPromise.get_future();

  m_getNeighboursPromises.emplace(requestId, std::move(getNeighboursPromise));
  m_getNeighboursFutures.emplace(requestId, std::move(getNeighboursFuture));

  log("getNeighbours node to query " + nodeToQuery.toString());

  if (nodeToQuery == m_id)
  {
    log("handling getNeigbours locally");
    Neighbours myNeighbours;
    myNeighbours.hasPredecessor = m_hasPredecessor;
    myNeighbours.predecessor = m_predecessor;
    myNeighbours.successor = m_successor;

    m_getNeighboursPromises.find(requestId)->second.set_value(myNeighbours);
    return requestId;
  }

  PendingMessageResponse pending;
  pending.m_type = MessageType::CHORD_GET_NEIGHBOURS_RESPONSE;
  pending.m_nodeId = nodeToQuery;
  pending.m_hasChain = false;

  log("created a pending response for " + pending.m_nodeId.toString());

  m_pendingResponses.emplace(requestId, pending);

  GetNeighboursMessage message{ CommsVersion::V1, m_id, requestId };

  log("sending GetNeighboursMessage");
  m_connectionManager->send(nodeToQuery, message);

  return requestId;
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

      std::function<bool()> work = [this, message]
      {
        log("handleJoinRequest task");
        handleJoinRequest(message);
        return true;
      };

      m_queue.putWork(work);
      break;
    }
    case MessageType::JOIN_RESPONSE:
    {
      log("received join response");
      JoinResponseMessage message{ CommsVersion::V1 };

      message.decode(std::move(encoded));

      std::function<bool()> work = [this, message]
      {
        log("handleJoinResponse task");
        handleJoinResponse(message);
        return true;
      };

      m_queue.putWork(work);
      break;
    }
    case MessageType::CHORD_FIND_SUCCESSOR:
    {
      log("received chord find successor request");
      FindSuccessorMessage message{ CommsVersion::V1 };

      message.decode(std::move(encoded));

      std::function<bool()> work = [this, message]
      {
        log("doFindSuccessor task");
        doFindSuccessor(message);
        return true;
      };

      m_queue.putWork(work);
      break;
    }

    case MessageType::CHORD_FIND_SUCCESSOR_RESPONSE:
    {
      log("received chord find successor response");
      FindSuccessorResponseMessage message{ CommsVersion::V1 };

      message.decode(std::move(encoded));

      std::function<bool()> work = [this, message]
      {
        log("handleFindSuccessorResponse task");
        handleFindSuccessorResponse(message);
        return true;
      };

      m_queue.putWork(work);
      break;
    }

    case MessageType::CHORD_NOTIFY:
    {
      log("received chord notify message");
      NotifyMessage message{ CommsVersion::V1 };

      message.decode(std::move(encoded));

      std::function<bool()> work = [this, message]
      {
        log("handleNotify task");
        handleNotify(message);
        return true;
      };

      m_queue.putWork(work);
      break;
    }

    case MessageType::CHORD_GET_NEIGHBOURS:
    {
      log("received chord get neighbours request") ;
      GetNeighboursMessage message{ CommsVersion::V1 };

      message.decode(std::move(encoded));

      std::function<bool()> work = [this, message]
      {
        log("handleGetNeighbours task");
        handleGetNeighbours(message);
        return true;
      };

      m_queue.putWork(work);
      break;
    }

    case MessageType::CHORD_GET_NEIGHBOURS_RESPONSE:
    {
      log("received chord get neighbours response") ;
      GetNeighboursResponseMessage message{ CommsVersion::V1 };

      message.decode(std::move(encoded));

      std::function<bool()> work = [this, message]
      {
        log("handleGetNeighboursResponse task");
        handleGetNeighboursResponse(message);
        return true;
      };

      m_queue.putWork(work);
      break;
    }

    case MessageType::CONNECT:
    {
      log("received connect message") ;
      ConnectMessage message{ CommsVersion::V1 };

      message.decode(std::move(encoded));

      std::function<bool()> work = [this, message]
      {
        log("handleConnectMessage task");
        handleConnectMessage(message);
        return true;
      };

      m_queue.putWork(work);
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
  if (not m_hasPredecessor ||
      (containedInOpenInterval(m_predecessor, m_id, message.nodeId())))
  {
    log("pred " + m_predecessor.toString() + " id " + m_id.toString() + " mess " + message.nodeId().toString());
    m_predecessor = message.nodeId();
    m_hasPredecessor = true;
    log("Notify: predecessor set to: " + m_predecessor.toString());
  }
}

void ChordNode::handleGetNeighbours(const GetNeighboursMessage& message)
{
  GetNeighboursResponseMessage response{ CommsVersion::V1 };

  if (m_hasPredecessor)
  {
    log("get neighbours with precessor");
    response = GetNeighboursResponseMessage{ CommsVersion::V1, m_successor, m_predecessor, m_id, message.requestId() };
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

void ChordNode::sendConnect(const NodeId& nodeId)
{
  sendConnect(nodeId, m_connectionManager->ip());
}

void ChordNode::sendConnect(const NodeId& nodeId, uint32_t ip)
{
  ConnectMessage message{ CommsVersion::V1, m_id, ip };

  m_connectionManager->send(nodeId, message);
}

void ChordNode::handleConnectMessage(const ConnectMessage& message)
{
  m_connectionManager->insert(message.nodeId(), message.ip(), 0);
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

void ChordNode::fixFingers()
{
  m_fingerTable.m_next++;

  log("Fixing finger " + std::to_string(m_fingerTable.m_next));

  if (m_fingerTable.m_next >= 160)
  {
    m_fingerTable.m_next = 0;
  }

  auto requestId = findSuccessor(m_fingerTable.m_fingers[m_fingerTable.m_next].m_end);

  std::function<bool()> checkFutureTask = [this, requestId, tableIndex = m_fingerTable.m_next] () -> bool
  {
    auto it = m_findSuccessorFutures.find(requestId);

    // if the future can't be found then do no load this task again.
    if (it == m_findSuccessorFutures.end())
    {
      log("Fix fingers: could not find future for this request");
      return true;
    }

    auto futureStatus = it->second.wait_for(std::chrono::milliseconds{20});

    if (futureStatus != std::future_status::ready)
    {
      return false;
    }
    
    m_fingerTable.m_fingers[tableIndex].m_nodeId = it->second.get();
    log("got successor, finger " + std::to_string(tableIndex) + " nodeId set to " + m_fingerTable.m_fingers[tableIndex].m_nodeId.toString());

    return true;
  };

  m_queue.putWork(checkFutureTask);
}

void ChordNode::stabilise()
{
  log("stabilise");

  auto requestId = getNeighbours(m_successor);

  std::function<bool()> checkFutureTask = [this, requestId] ()
  {
    auto it = m_getNeighboursFutures.find(requestId);

    if (it == m_getNeighboursFutures.end()) return true;

    auto result = it->second.wait_for(std::chrono::milliseconds{20});

    if (result != std::future_status::ready) return false;

    Neighbours successorNeighbours = it->second.get();


    if (successorNeighbours.hasPredecessor &&
        containedInOpenInterval(m_id, m_successor, successorNeighbours.predecessor))
    {
      m_successor = successorNeighbours.predecessor;
    }

    notify(m_successor);
    return true;
  };

  m_queue.putWork(checkFutureTask);

}

void ChordNode::log(const std::string& message)
{
  std::cout << "[" << m_nodeName << "-" << m_id.toString() << "] ChordNode: " << message << std::endl;
}

} // namespace chord

