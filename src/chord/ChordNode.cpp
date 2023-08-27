#include "ChordNode.h"
#include "ChordMessaging.h"
#include "../comms/CommsCoder.h"

#include <algorithm>
#include <arpa/inet.h>

namespace odd::chord {

ChordNode::ChordNode(const std::string& nodeName,
                     const std::string& ip,
                     uint16_t port,
                     const ConnectionManagerFactory& connectionManagerFactory,
                     std::unique_ptr<logging::Logger> logger)
  : m_nodeName(nodeName),
    m_ipAddress{convertIpAddressToInteger(ip)},
    m_id(m_ipAddress),
    m_predecessor{},
    m_successor{m_id},
    m_port{port},
    m_connectionManager(connectionManagerFactory(NodeId{m_ipAddress}, m_ipAddress, port)),
    m_logger(std::move(logger)),
    m_logPrefix(nodeName + " - " + m_id.toString() + ": "),
    m_running(true)
{
  initialiseFingerTable(m_fingerTable, m_id);

  io::tcp::OnReceiveCallback onReceiveCallback = [this] (uint8_t* message, std::size_t messageLength)
  {
    receive(message, messageLength);
  };

  m_connectionManager->registerReceiveHandler(onReceiveCallback);

  m_workThread = std::thread{&ChordNode::workThread, this};
}

ChordNode::~ChordNode()
{
  m_running = false;
  m_connectionManager->stop();
  m_workThread.join();
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
    m_logger->log(m_logPrefix + "Running join task");
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

    m_logger->log(m_logPrefix + "JoinTask: sending JoinMessage");

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

      m_logger->log(m_logPrefix + "first findSuccessor has found successor, " + m_successor.toString());

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
  m_logger->log(m_logPrefix + "receiving message");
  EncodedMessage encoded{message, messageLength};

  handleReceivedMessage(std::move(encoded));
}

void ChordNode::workThread()
{
  while (m_running)
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
  m_logger->log(m_logPrefix + "finding successor for " + message.queryNodeId().toString());

  if (containedInLeftOpenInterval(m_id, m_successor, message.queryNodeId()))
  {
    m_logger->log(m_logPrefix + "successor found for " + message.queryNodeId().toString());

    // If this is the case then we can start returning
    FindSuccessorResponseMessage response{ CommsVersion::V1, m_successor, m_id, m_connectionManager->ip(m_successor), message.requestId() };

    m_logger->log(m_logPrefix + "sending FindSuccessorResponse");

    if (not m_connectionManager->send(message.sourceNodeId(), response))
    {
      findIp(message.sourceNodeId());
    }

    return;
  }

  auto nodeId = closestPrecedingFinger(message.queryNodeId());

  m_logger->log(m_logPrefix + "could not find successor for " + message.queryNodeId().toString());
  m_logger->log(m_logPrefix + "forwarding message to " + nodeId.toString());

  auto requestId = getNextAvailableRequestId();

  PendingMessageResponse pending;
  pending.m_type = MessageType::CHORD_FIND_SUCCESSOR_RESPONSE;
  pending.m_nodeId = nodeId;
  pending.m_hasChain = true;
  pending.m_chainingDestination = message.sourceNodeId();

  m_pendingResponses.emplace(requestId, pending);

  FindSuccessorMessage messageToForward{ CommsVersion::V1, message.queryNodeId(), m_id, requestId };

  m_logger->log(m_logPrefix + "sending FindSuccessorMessage");
  m_connectionManager->send(nodeId, messageToForward);
  if (not m_connectionManager->send(nodeId, messageToForward))
  {
    findIp(nodeId);
  }
}

void ChordNode::handleFindSuccessorResponse(const FindSuccessorResponseMessage& message)
{
  auto it = m_pendingResponses.find(message.requestId());

  if (it == m_pendingResponses.end())
  {
    m_logger->log(m_logPrefix + "Unexpected find successor response with ID: " + std::to_string(message.requestId()) + " from node: " + message.sourceNodeId().toString());
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

    m_logger->log(m_logPrefix + "sending FindSuccessorResponse");

    if (not m_connectionManager->send(it->second.m_chainingDestination, messageToForward))
    {
      findIp(it->second.m_chainingDestination);
    }

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

  m_logger->log(m_logPrefix + "m_successor has been set to " + m_successor.toString());
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

  m_logger->log(m_logPrefix + "sending FindSuccessorMessage");

  if (not m_connectionManager->send(nodeToQuery, message))
  {
    findIp(nodeToQuery);
  }

  return requestId;
}

uint32_t ChordNode::getNeighbours(const NodeId& nodeToQuery)
{
  auto requestId = getNextAvailableRequestId();

  m_logger->log(m_logPrefix + "get neighbours, request ID " + std::to_string(requestId));

  std::promise<Neighbours> getNeighboursPromise;
  std::future<Neighbours> getNeighboursFuture = getNeighboursPromise.get_future();

  m_getNeighboursPromises.emplace(requestId, std::move(getNeighboursPromise));
  m_getNeighboursFutures.emplace(requestId, std::move(getNeighboursFuture));

  m_logger->log(m_logPrefix + "getNeighbours node to query " + nodeToQuery.toString());

  if (nodeToQuery == m_id)
  {
    m_logger->log(m_logPrefix + "handling getNeigbours locally");
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

  m_logger->log(m_logPrefix + "created a pending response for " + pending.m_nodeId.toString());

  m_pendingResponses.emplace(requestId, pending);

  GetNeighboursMessage message{ CommsVersion::V1, m_id, requestId };

  m_logger->log(m_logPrefix + "sending GetNeighboursMessage");

  if (not m_connectionManager->send(nodeToQuery, message))
  {
    findIp(nodeToQuery);
  }

  return requestId;
}

void ChordNode::notify(const NodeId& nodeId)
{
  NotifyMessage message{ CommsVersion::V1, m_id };

  m_logger->log(m_logPrefix + "sending NotifyMessage");

  if (not m_connectionManager->send(nodeId, message))
  {
    findIp(nodeId);
  }
}

const NodeId &ChordNode::closestPrecedingFinger(const NodeId &id)
{
  for (int i = 159; i >= 0; i--)
  {
    if (containedInOpenInterval(m_id, id, m_fingerTable.m_fingers[i].m_nodeId))
    {
      m_logger->log(m_logPrefix + "closest preceding finger " + m_fingerTable.m_fingers[i].m_nodeId.toString());
      return m_fingerTable.m_fingers[i].m_nodeId;
    }
  }

  m_logger->log(m_logPrefix + "not closest preceding finger found returning id " + m_id.toString());
  return m_id;
}

void ChordNode::handleReceivedMessage(EncodedMessage&& encoded)
{
  // Ignore comms version for now, this will probably be handled differently at a later time

  // get the message type, which is a uint32_t starting at encoded.message[2]
  uint32_t type;
  decodeSingleValue(&encoded.m_message[2], &type);

  switch (static_cast<MessageType>(type))
  {
    case MessageType::JOIN:
    {
      m_logger->log(m_logPrefix + "received join request");
      JoinMessage message{ CommsVersion::V1 };

      message.decode(std::move(encoded));

      std::function<bool()> work = [this, message]
      {
        handleJoinRequest(message);
        return true;
      };

      m_queue.putWork(work);
      break;
    }
    case MessageType::JOIN_RESPONSE:
    {
      m_logger->log(m_logPrefix + "received join response");
      JoinResponseMessage message{ CommsVersion::V1 };

      message.decode(std::move(encoded));

      std::function<bool()> work = [this, message]
      {
        handleJoinResponse(message);
        return true;
      };

      m_queue.putWork(work);
      break;
    }
    case MessageType::CHORD_FIND_SUCCESSOR:
    {
      m_logger->log(m_logPrefix + "received chord find successor request");
      FindSuccessorMessage message{ CommsVersion::V1 };

      message.decode(std::move(encoded));

      std::function<bool()> work = [this, message]
      {
        doFindSuccessor(message);
        return true;
      };

      m_queue.putWork(work);
      break;
    }

    case MessageType::CHORD_FIND_SUCCESSOR_RESPONSE:
    {
      m_logger->log(m_logPrefix + "received chord find successor response");
      FindSuccessorResponseMessage message{ CommsVersion::V1 };

      message.decode(std::move(encoded));

      std::function<bool()> work = [this, message]
      {
        handleFindSuccessorResponse(message);
        return true;
      };

      m_queue.putWork(work);
      break;
    }

    case MessageType::CHORD_NOTIFY:
    {
      m_logger->log(m_logPrefix + "received chord notify message");
      NotifyMessage message{ CommsVersion::V1 };

      message.decode(std::move(encoded));

      std::function<bool()> work = [this, message]
      {
        handleNotify(message);
        return true;
      };

      m_queue.putWork(work);
      break;
    }

    case MessageType::CHORD_GET_NEIGHBOURS:
    {
      m_logger->log(m_logPrefix + "received chord get neighbours request") ;
      GetNeighboursMessage message{ CommsVersion::V1 };

      message.decode(std::move(encoded));

      std::function<bool()> work = [this, message]
      {
        handleGetNeighbours(message);
        return true;
      };

      m_queue.putWork(work);
      break;
    }

    case MessageType::CHORD_GET_NEIGHBOURS_RESPONSE:
    {
      m_logger->log(m_logPrefix + "received chord get neighbours response") ;
      GetNeighboursResponseMessage message{ CommsVersion::V1 };

      message.decode(std::move(encoded));

      std::function<bool()> work = [this, message]
      {
        handleGetNeighboursResponse(message);
        return true;
      };

      m_queue.putWork(work);
      break;
    }

    case MessageType::CONNECT:
    {
      m_logger->log(m_logPrefix + "received connect message") ;
      ConnectMessage message{ CommsVersion::V1 };

      message.decode(std::move(encoded));

      std::function<bool()> work = [this, message]
      {
        handleConnectMessage(message);
        return true;
      };

      m_queue.putWork(work);
      break;
    }

    case MessageType::FIND_IP:
    {
      m_logger->log(m_logPrefix + "received find ip message") ;
      FindIpMessage message{ CommsVersion::V1 };

      message.decode(std::move(encoded));

      std::function<bool()> work = [this, message]
      {
        handleFindIp(message);
        return true;
      };

      m_queue.putWork(work);
      break;
    }
    default:
    {
      m_logger->log(m_logPrefix + "received unknown message type: 0x");
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

  m_logger->log(m_logPrefix + "sending JoinResponse");
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
    m_logger->log(m_logPrefix + "pred " + m_predecessor.toString() + " id " + m_id.toString() + " mess " + message.nodeId().toString());
    m_predecessor = message.nodeId();
    m_hasPredecessor = true;
    m_logger->log(m_logPrefix + "Notify: predecessor set to: " + m_predecessor.toString());
  }
}

void ChordNode::handleGetNeighbours(const GetNeighboursMessage& message)
{
  GetNeighboursResponseMessage response{ CommsVersion::V1 };

  if (m_hasPredecessor)
  {
    m_logger->log(m_logPrefix + "get neighbours with precessor");
    response = GetNeighboursResponseMessage{ CommsVersion::V1, m_successor, m_predecessor, m_id, message.requestId() };
  }
  else
  {
    response = GetNeighboursResponseMessage{ CommsVersion::V1, m_successor, m_id, message.requestId() };
  }

  m_logger->log(m_logPrefix + "sending GetNeigboursResponse");
  if (not m_connectionManager->send(message.sourceNodeId(), response))
  {
    findIp(message.sourceNodeId());
  }
}

void ChordNode::handleGetNeighboursResponse(const GetNeighboursResponseMessage& message)
{
  auto it = m_pendingResponses.find(message.requestId());

  if (it == m_pendingResponses.end())
  {
    m_logger->log(m_logPrefix + "Unexpected get neighbours response from node: " + message.sourceNodeId().toString() + ", id " + std::to_string(message.requestId()));
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

void ChordNode::sendConnect(const NodeId& destination)
{
  m_logger->log(m_logPrefix + "sending self connect");
  sendConnect(destination, m_id, m_connectionManager->ip());
}

void ChordNode::sendConnect(const NodeId& destination, const NodeId& nodeId, uint32_t ip)
{
  ConnectMessage message{ CommsVersion::V1, nodeId, ip };

  m_logger->log(m_logPrefix + "sending connect message to " + destination.toString() + " sending node " + nodeId.toString() + ", ip " + std::to_string(ip));

  m_connectionManager->send(destination, message);
}

void ChordNode::handleConnectMessage(const ConnectMessage& message)
{
  m_connectionManager->insert(message.nodeId(), message.ip(), 0);
}

void ChordNode::findIp(const NodeId& nodeId)
{
  FindIpMessage message{ CommsVersion::V1, nodeId, m_id, m_connectionManager->ip(), 5 };

  m_logger->log(m_logPrefix + "sending find ip for node " + nodeId.toString() + " our ip is " + std::to_string(m_connectionManager->ip()));

  m_connectionManager->broadcast(message);
}

void ChordNode::handleFindIp(const FindIpMessage& message)
{
  if (message.timeToLive() == 0) return;

  uint32_t ip = m_connectionManager->ip(message.nodeId());

  if (ip == 0)
  {
    m_logger->log(m_logPrefix + "could not find ip address, broadcasting again, ttl " + std::to_string(message.timeToLive()));
    FindIpMessage messageToForward{ CommsVersion::V1,
                                    message.nodeId(),
                                    message.sourceNodeId(),
                                    message.sourceNodeIp(),
                                    message.timeToLive() - 1 };

    m_connectionManager->broadcast(messageToForward);

    return;
  }

  m_logger->log(m_logPrefix + "found ip address for " + message.nodeId().toString());
  m_logger->log(m_logPrefix + "inserting " + message.sourceNodeId().toString() + ", " + std::to_string(message.sourceNodeIp()));

  m_connectionManager->insert(message.sourceNodeId(), message.sourceNodeIp(), 0);

  sendConnect(message.sourceNodeId(), message.nodeId(), ip);
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

  m_logger->log(m_logPrefix + "Fixing finger " + std::to_string(m_fingerTable.m_next));

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
      m_logger->log(m_logPrefix + "Fix fingers: could not find future for this request");
      return true;
    }

    auto futureStatus = it->second.wait_for(std::chrono::milliseconds{20});

    if (futureStatus != std::future_status::ready)
    {
      return false;
    }
    
    m_fingerTable.m_fingers[tableIndex].m_nodeId = it->second.get();
    m_logger->log(m_logPrefix + "got successor, finger " + std::to_string(tableIndex) + " nodeId set to " + m_fingerTable.m_fingers[tableIndex].m_nodeId.toString());

    return true;
  };

  m_queue.putWork(checkFutureTask);
}

void ChordNode::stabilise()
{
  m_logger->log(m_logPrefix + "stabilise");

  auto requestId = getNeighbours(m_successor);

  std::function<bool()> checkFutureTask = [this, requestId] ()
  {
    auto it = m_getNeighboursFutures.find(requestId);

    if (it == m_getNeighboursFutures.end()) return true;

    auto result = it->second.wait_for(std::chrono::milliseconds{20});

    if (result != std::future_status::ready) return false;

    m_logger->log(m_logPrefix + "stabilise - got neighbours from successor");
    Neighbours successorNeighbours = it->second.get();


    if (successorNeighbours.hasPredecessor &&
        containedInOpenInterval(m_id, m_successor, successorNeighbours.predecessor))
    {
      m_successor = successorNeighbours.predecessor;
    }

    m_logger->log(m_logPrefix + "stabilise - notifying successor " + m_successor.toString());

    notify(m_successor);
    return true;
  };

  m_queue.putWork(checkFutureTask);

}

void ChordNode::log(const std::string& message)
{
  std::cout << "[" << m_nodeName << "-" << m_id.toString() << "] ChordNode: " << message << std::endl;
}

} // namespace odd::chord

