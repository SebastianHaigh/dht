#include "ChordNode.h"
#include "ChordMessaging.h"
#include "../comms/CommsCoder.h"
#include "NodeId.h"

#include <algorithm>
#include <arpa/inet.h>
#include <chrono>
#include <functional>

namespace chord {

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
    m_running(true),
    m_async(*m_connectionManager)
{
  ::chord::initialiseFingerTable(m_fingerTable, m_id);

  tcp::OnReceiveCallback onReceiveCallback = [this] (uint8_t* message, std::size_t messageLength)
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

  std::function<void(Message&&)> onJoinComplete = [this, knownNodeId] (Message &&)
  {
    findSuccessor(knownNodeId, m_id, [this] (const NodeId& successor) { m_successor = successor; });
  };

  m_logger->log(m_logPrefix + "JoinTask: sending JoinMessage");
  m_async.post(knownNodeId, 
               JoinMessage{ CommsVersion::V1, m_connectionManager->ip(), requestId },
               onJoinComplete,
               requestId);


  /* auto ip = convertIpAddressToInteger(knownNodeIpAddress); */
  /* NodeId knownNodeId{ ip }; */

  /* std::function<bool()> findSuccessorTask = [this, knownNodeId] () -> bool */
  /* { */
  /*   auto requestId = findSuccessor(knownNodeId, m_id); */

  /*   std::function<bool()> checkFutureTask = [this, requestId] () -> bool */
  /*   { */
  /*     auto it = m_findSuccessorFutures.find(requestId); */

  /*     // if the future can't be found then do not load this task again. */
  /*     if (it == m_findSuccessorFutures.end()) return true; */

  /*     auto futureStatus = it->second.wait_for(std::chrono::milliseconds{20}); */

  /*     // future not ready, run the task again */
  /*     if (futureStatus != std::future_status::ready) return false; */

  /*     m_successor = it->second.get(); */

  /*     m_logger->log(m_logPrefix + "first findSuccessor has found successor, " + m_successor.toString()); */

  /*     // successor found, no need to run again */
  /*     return true; */
  /*   }; */

    /* m_queue.putWork(checkFutureTask); */

    /* return true; */
  /* }; */

  /* m_queue.putWork(findSuccessorTask); */
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

    sendFindSuccessorResponse(message.sourceNodeId(), m_successor, message.requestId());

    return;
  }

  std::function<void(const NodeId&)> onSuccessorFound = [this, nodeId = message.sourceNodeId(), id = message.requestId()] (const NodeId& successor)
  {
    m_connectionManager->send(nodeId,
                              FindSuccessorResponseMessage{CommsVersion::V1,
                                                           successor,
                                                           m_id,
                                                           m_connectionManager->ip(),
                                                           id});
  };

  // This call is going to eventally going to send out an RPC that returns the succesor and send as response back to the node we got this request from in the first place.
  findSuccessor(message.queryNodeId(), onSuccessorFound);
}

void ChordNode::handleFindSuccessorResponse(FindSuccessorResponseMessage&& message)
{
  auto requestId = message.requestId();
  m_async.handleReceived(std::move(message), requestId);
}

void ChordNode::findSuccessor(const NodeId& hash,
                              std::function<void(const NodeId&)> onFound)
{
  return findSuccessor(closestPrecedingFinger(hash), hash, onFound);
}

void ChordNode::findSuccessor(const NodeId& nodeToQuery,
                              const NodeId& hash,
                              std::function<void(const NodeId&)> onFound)
{
  if (nodeToQuery == m_id)
  {
    onFound(m_successor);
    return;
  }

  // Build the find successor rpc

  std::function<void(Message&&)> completionHandler = [onFound] (Message&& message) mutable
  { 
    auto& r = static_cast<FindSuccessorResponseMessage&>(message);

    onFound(r.nodeId());
  };

  auto requestId = getNextAvailableRequestId();
  FindSuccessorMessage message{ CommsVersion::V1, hash, m_id, requestId };

  // If we don't have the ip for the node then we will need to find it before sending the find
  // successor rpc.
  auto ip = m_connectionManager->ip(nodeToQuery);

  if (ip == 0)
  {
    findIpAndSendMessage(nodeToQuery,
                         std::move(message),
                         std::move(completionHandler),
                         requestId);

    return;
  }

  // We already have the IP so we can send the rpc straight away
  m_async.post(nodeToQuery,
               std::move(message),
               std::move(completionHandler),
               requestId);
}

void ChordNode::sendFindSuccessorResponse(const NodeId& dest,
                                          const NodeId& successorNodeId,
                                          uint32_t requestId)
{
  auto ip = m_connectionManager->ip(dest);

  FindSuccessorResponseMessage message{ CommsVersion::V1,
                                        successorNodeId,
                                        m_id,
                                        m_connectionManager->ip(),
                                        requestId };

  if (ip == 0)
  {
    findIpAndSendMessage(dest,
                         std::move(message),
                         [] (Message&&) {},
                         requestId);

    return;
  }
  
  // We already have the IP so we can send the response straight away
  m_async.post(dest,
               std::move(message),
               [] (Message&&) {},
               requestId);
}

void ChordNode::getNeighbours(const NodeId& nodeToQuery,
                              GetNeighboursCallback onGot)
{
  if (nodeToQuery == m_id)
  {
    m_logger->log(m_logPrefix + "handling getNeigbours locally");
    Neighbours myNeighbours;
    myNeighbours.hasPredecessor = m_hasPredecessor;
    myNeighbours.predecessor = m_predecessor;
    myNeighbours.successor = m_successor;

    onGot(myNeighbours);
    return;
  }
  auto requestId = getNextAvailableRequestId();

  m_logger->log(m_logPrefix + "get neighbours, request ID " + std::to_string(requestId));
  m_logger->log(m_logPrefix + "getNeighbours node to query " + nodeToQuery.toString());

  GetNeighboursMessage message{ CommsVersion::V1, m_id, requestId };

  std::function<void(Message&&)> completionHandler = [onGot] (Message&& message)
  {
    auto& getNeighboursResponse = static_cast<GetNeighboursResponseMessage&>(message);

    onGot({getNeighboursResponse.successor(),
           getNeighboursResponse.predecessor(),
           getNeighboursResponse.hasPredecessor()});
  };

  m_logger->log(m_logPrefix + "sending GetNeighboursMessage");

  sendMessage(nodeToQuery,
              GetNeighboursMessage{ CommsVersion::V1, m_id, requestId },
              completionHandler,
              requestId);
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

      handleJoinResponse(std::move(message));
      break;
    }
    case MessageType::CHORD_FIND_SUCCESSOR:
    {
      m_logger->log(m_logPrefix + "received chord find successor request");
      FindSuccessorMessage message{ CommsVersion::V1 };

      message.decode(std::move(encoded));

      doFindSuccessor(message);

      break;
    }

    case MessageType::CHORD_FIND_SUCCESSOR_RESPONSE:
    {
      m_logger->log(m_logPrefix + "received chord find successor response");
      FindSuccessorResponseMessage message{ CommsVersion::V1 };

      message.decode(std::move(encoded));

      handleFindSuccessorResponse(std::move(message));

      break;
    }

    case MessageType::CHORD_NOTIFY:
    {
      m_logger->log(m_logPrefix + "received chord notify message");
      NotifyMessage message{ CommsVersion::V1 };

      message.decode(std::move(encoded));

      handleNotify(message);

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

void ChordNode::handleJoinResponse(JoinResponseMessage&& message)
{
  auto requestId = message.requestId();
  m_async.handleReceived(std::move(message), requestId);
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

void ChordNode::handleGetNeighboursResponse(GetNeighboursResponseMessage&& message)
{
  auto requestId = message.requestId();
  m_async.handleReceived(message, requestId);
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

void ChordNode::sendMessage(const NodeId& dest,
                            Message&& message,
                            std::function<void(Message&&)> completionHandler,
                            uint32_t requestId)
{
  auto ip = m_connectionManager->ip(dest);

  if (ip == 0)
  {
    findIpAndSendMessage(dest,
                         std::move(message),
                         std::move(completionHandler),
                         requestId);

    return;
  }

  // We already have the IP so we can send the rpc straight away
  m_async.post(dest,
               std::move(message),
               std::move(completionHandler),
               requestId);
}

void ChordNode::findIpAndSendMessage(const NodeId& dest,
                                     Message&& message,
                                     std::function<void(Message&&)> completionHandler,
                                     uint32_t requestId)
{
    // find the ip and post find successor pending ip being found
    //   The findIp call will go out,
    //   once this node receives the IP of nodeToQuery the async engine will send the pending message

    // TODO (haigh) a pending message like this should have a time out
    m_async.postPendingEvent(dest,
                             std::move(message),
                             std::move(completionHandler),
                             [this, dest] { return (m_connectionManager->ip(dest) != 0); },
                             requestId);

    findIp(dest);
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

  std::function<void(const NodeId&)> onFound = [this, tableIndex = m_fingerTable.m_next] (const NodeId& successor)
  {
    m_fingerTable.m_fingers[tableIndex].m_nodeId = successor;
    m_logger->log(m_logPrefix + "got successor, finger " + std::to_string(tableIndex) + " nodeId set to " + m_fingerTable.m_fingers[tableIndex].m_nodeId.toString());
  };

  findSuccessor(m_fingerTable.m_fingers[m_fingerTable.m_next].m_end, onFound);
}

void ChordNode::stabilise()
{
  m_logger->log(m_logPrefix + "stabilise");

  GetNeighboursCallback callback = [this] (const Neighbours& neighbours)
  {
    if (neighbours.hasPredecessor &&
        containedInOpenInterval(m_id, m_successor, neighbours.predecessor))
    {
      m_successor = neighbours.predecessor;
    }

    m_logger->log(m_logPrefix + "stabilise - notifying successor " + m_successor.toString());

    notify(m_successor);
  };

  getNeighbours(m_successor, callback);
}

void ChordNode::log(const std::string& message)
{
  std::cout << "[" << m_nodeName << "-" << m_id.toString() << "] ChordNode: " << message << std::endl;
}

} // namespace chord

