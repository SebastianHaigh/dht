#ifndef CHORD_NODE_H_
#define CHORD_NODE_H_

#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <future>
#include <mutex>
#include <optional>
#include <queue>
#include <unordered_map>

#include "../comms/Comms.h"
#include "../logger/Logger.h"

#include "ChordMessaging.h"
#include "NodeId.h"
#include "FingerTable.h"
#include "ConnectionManager.h"

namespace chord {

static constexpr uint8_t NULL_NODE_ID[20] = { 0 };

using IpAddress = std::string;
using ConnectionManagerFactory = std::function<std::unique_ptr<ConnectionManager_I>(const NodeId&, uint32_t, uint16_t)>;

struct RPCHandler
{
  int m_id;
  std::function<void()> m_rpc;
};

class FutureMessageSender
{
  public:
    FutureMessageSender(ConnectionManager_I& connectionManager,
                        const NodeId& nodeId,
                        EncodedMessage&& message)
      : m_c(connectionManager),
        m_n(nodeId),
        m_m(std::move(message)),
        m_called(false)
    {
    }

    FutureMessageSender(FutureMessageSender&& rhs) noexcept
      : m_c(rhs.m_c),
        m_n(rhs.m_n),
        m_m(std::move(rhs.m_m)),
        m_called(rhs.m_called)
    {
    }

    FutureMessageSender& operator=(FutureMessageSender&& rhs) noexcept
    {
      m_c = rhs.m_c;
      m_n = rhs.m_n;
      m_m = std::move(rhs.m_m);
      m_called = rhs.m_called;
      return *this;
    }

    void send()
    {
      if (not m_called)
      {
        m_c.sendEncoded(m_n, std::move(m_m));
        m_called = true;
      }
    }

  private:
    ConnectionManager_I& m_c;
    NodeId m_n;
    EncodedMessage m_m;
    bool m_called;
};

class FutureHandler
{
  public:
    explicit FutureHandler(std::function<bool()> onFuture)
      : m_onFuture(std::move(onFuture))
    {
    }

    FutureHandler(const FutureHandler&) = delete;
    FutureHandler(FutureHandler&& rhs) noexcept
      : m_onFuture(std::move(rhs.m_onFuture))
    {
    }

    FutureHandler& operator=(const FutureHandler&) = delete;
    FutureHandler& operator=(FutureHandler&& rhs) noexcept
    {
      m_onFuture = std::move(rhs.m_onFuture);
      return *this;
    }

    bool doFuture()
    {
      return m_onFuture();
    }

  private:
    std::function<bool()> m_onFuture;
};

class AsyncEngine
{
  public:
    explicit AsyncEngine(ConnectionManager_I& connectionManager)
      : m_connectionManager(connectionManager)
    {
    }

    // here we need to post the rpc that we want to send.
    // we need to provide a completion handler that tells the async engine what to do once it receives
    // a response
    void post(const NodeId& dest,
              Message&& rpc, // TODO (haigh) should this be a universal ref? Move only semantics
              std::function<void(Message&& response)> completionHandler,
              uint32_t requestId)
    {
      // Add the handler to the map
      m_completionHandlers.emplace(requestId, std::move(completionHandler));

      // should the connection manager get the message or should we encode it first?
      m_connectionManager.send(dest, rpc);
    }

    void postPendingEvent(const NodeId& dest,
                          Message&& rpc,
                          std::function<void(Message&& response)> completionHandler,
                          std::function<bool()> eventHappened,
                          uint32_t requestId)
    {
      m_completionHandlers.emplace(requestId, completionHandler);
      m_events.emplace(requestId, eventHappened);

      m_pendingMessages.emplace(requestId, 
                                FutureMessageSender{m_connectionManager,
                                                    dest,
                                                    rpc.encode()});
    }

    void postPendingFuture(std::function<bool()> onFuture)
    {
      m_pendingFuture.emplace_back(std::move(onFuture));
    }

    void runOnePending()
    {
      for (const auto& event : m_events)
      {
        if (event.second())
        {
          // The event has happened, so now we can send its message
          auto pending_it = m_pendingMessages.find(event.first);

          if (pending_it == m_pendingMessages.end()) break; // TODO (haigh) does this need to be handled?

          pending_it->second.send();
          // TODO (haigh) both the event and the pending message should now be erased.
          m_pendingMessages.erase(pending_it);
        }
      }

      auto future_it = m_pendingFuture.begin();
      while (future_it != m_pendingFuture.end())
      {
        if (future_it->doFuture())
        {
          future_it = m_pendingFuture.erase(future_it);
          continue;
        }
        future_it++;
      }

    }

    void handleReceived(Message&& rpc,
                        uint32_t requestId)
    {
      auto handler_it = m_completionHandlers.find(requestId);

      if (handler_it == m_completionHandlers.end()) return;

      handler_it->second(std::move(rpc));

      m_completionHandlers.erase(handler_it);
    }

  private:
    ConnectionManager_I& m_connectionManager;

    // When you register your rpc, you also register a completion handler,
    // this is a task to be executed once the rpc result returns;
    std::unordered_map<uint32_t, std::function<void(Message&&)>> m_completionHandlers;
    std::unordered_map<uint32_t, std::function<bool()>> m_events;
    std::unordered_map<uint32_t, FutureMessageSender> m_pendingMessages;
    std::vector<FutureHandler> m_pendingFuture;
};

class WorkThreadQueue
{
  public:
    WorkThreadQueue() = default;

    bool putWork(std::function<bool()> workItem)
    {
      size_t putIndex = m_tail.load();
      size_t nextIndex = (putIndex + 1) % 100;

      while (nextIndex == m_head.load())
      {
        return false;
      }

      m_workItems[putIndex] = std::move(workItem);
      m_tail.store(nextIndex);
      return true;
    }

    void doNextWork()
    {
      size_t readIndex = m_head.load();
      size_t nextIndex = (readIndex + 1) % 100;

      while (readIndex == m_tail.load())
      {
        return;
      }

      if (not m_workItems[readIndex]())
      {
        putWork(std::move(m_workItems[readIndex]));
        m_workItems[readIndex] = nullptr;
      }
      m_head.store(nextIndex);
    }

    bool hasWork()
    {
      return (m_head.load(std::memory_order_relaxed) != m_tail.load(std::memory_order_relaxed));
    }

  private:
    std::array<std::function<bool()>, 100> m_workItems;
    size_t m_ringSize = 100;
    std::atomic<size_t> m_head{0};
    std::atomic<size_t> m_tail{0};
};

class ChordNode
{
  public:
    ChordNode(const std::string& nodeName,
              const std::string& ip,
              uint16_t port,
              const ConnectionManagerFactory& factory,
              std::unique_ptr<logging::Logger> logger);

    ~ChordNode();
    void create();
    void join(const std::string &knownNodeIpAddress);
    const NodeId& getId() const;

    const NodeId& getPredecessorId() const;
    const NodeId& getSuccessorId() const;

    const NodeId& closestPrecedingFinger(const NodeId& id);
    void receive(uint8_t* message, std::size_t messageLength);

  private:
    void log(const std::string& message);

    void doFindSuccessor(const FindSuccessorMessage& message);
    void handleFindSuccessorResponse(FindSuccessorResponseMessage&& message);

    void handleReceivedMessage(EncodedMessage&& encoded);

    void handleJoinRequest(const JoinMessage& message);
    void handleJoinResponse(JoinResponseMessage&& message);

    void handleNotify(const NotifyMessage& message);
    void handleGetNeighbours(const GetNeighboursMessage& message);
    void handleGetNeighboursResponse(GetNeighboursResponseMessage&& message);

    void sendConnect(const NodeId& destination);
    void sendConnect(const NodeId& destination, const NodeId& nodeId, uint32_t ip);
    void handleConnectMessage(const ConnectMessage& message);

    void findIp(const NodeId& nodeId);
    void handleFindIp(const FindIpMessage& message);

    void findIpAndSendMessage(const NodeId& dest,
                              Message&& message,
                              std::function<void(Message&&)> completionHandler,
                              uint32_t requestId);

    void sendMessage(const NodeId& dest,
                     Message&& message,
                     std::function<void(Message&&)> completionHandler,
                     uint32_t requestId);

    void fixFingers();

    void stabilise();

    void workThread();

    void findSuccessor(const NodeId& hash,
                       std::function<void(const NodeId&)> onFound);
    void findSuccessor(const NodeId& nodeToQuery,
                       const NodeId& hash,
                       std::function<void(const NodeId&)> onFound);
    void sendFindSuccessorResponse(const NodeId& dest,
                                   const NodeId& successorNodeId,
                                   uint32_t requestId);

    struct Neighbours
    {
      NodeId successor;
      NodeId predecessor;
      bool hasPredecessor;
    };
    using GetNeighboursCallback = std::function<void(const Neighbours&)>;
    void getNeighbours(const NodeId& nodeToQuery,
                       GetNeighboursCallback onGot);

    void notify(const NodeId& nodeId);

    static uint32_t convertIpAddressToInteger(const std::string& ipAddress);

    uint32_t getNextAvailableRequestId();

    const std::string m_nodeName;
    const uint32_t m_ipAddress;
    NodeId m_id;

    bool m_hasPredecessor = false;
    NodeId m_predecessor;
    NodeId m_successor;
    FingerTable m_fingerTable;
    const uint16_t m_port;

    std::unique_ptr<ConnectionManager_I> m_connectionManager;

    uint32_t m_requestIdCounter = 0;

    // Stores the details of a message this node is waiting for
    //   type of message and the ID of the node we are expecting to received the message from.
    struct PendingMessageResponse
    {
      MessageType m_type;
      NodeId m_nodeId;
      bool m_hasChain;
      NodeId m_chainingDestination;
    };

    std::unique_ptr<logging::Logger> m_logger;
    const std::string m_logPrefix;

    std::unordered_map<uint32_t, PendingMessageResponse> m_pendingResponses;
    std::unordered_map<uint32_t, std::promise<NodeId>> m_findSuccessorPromises;
    std::unordered_map<uint32_t, std::future<NodeId>> m_findSuccessorFutures;
    std::unordered_map<uint32_t, std::promise<Neighbours>> m_getNeighboursPromises;
    std::unordered_map<uint32_t, std::future<Neighbours>> m_getNeighboursFutures;
    std::promise<NodeId> m_joinPromise;

    WorkThreadQueue m_queue;
    std::future<NodeId> m_joinFuture;

    std::chrono::time_point<std::chrono::high_resolution_clock> m_lastManageTime;

    std::thread m_workThread;
    bool m_running;

    AsyncEngine m_async;
};

} // namespace chord

#endif // CHORD_NODE_H_

