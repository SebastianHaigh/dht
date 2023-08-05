#ifndef CHORD_NODE_H_
#define CHORD_NODE_H_

#include <future>
#include <optional>

#include "../comms/Comms.h"

#include "ChordMessaging.h"
#include "NodeId.h"
#include "FingerTable.h"
#include "ConnectionManager.h"

namespace chord {

static constexpr uint8_t NULL_NODE_ID[20] = { 0 };

using IpAddress = std::string;
using ConnectionManagerFactory = std::function<std::unique_ptr<ConnectionManager_I>(const NodeId&, uint32_t, uint16_t)>;

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
        putWork(std::move(m_workItems[readIndex]));
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
              const ConnectionManagerFactory& factory);

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
    void handleFindSuccessorResponse(const FindSuccessorResponseMessage& message);

    void initialiseFingerTable();
    void updateFingerTable(const ChordNode& node, uint16_t i);

    void handleReceivedMessage(const EncodedMessage& encoded);

    void handleJoinRequest(const JoinMessage& message);
    void handleJoinResponse(const JoinResponseMessage& message);

    void handleNotify(const NotifyMessage& message);
    void handleGetNeighbours(const GetNeighboursMessage& message);
    void handleGetNeighboursResponse(const GetNeighboursResponseMessage& message);

    void sendConnect(const NodeId& nodeId);
    void sendConnect(const NodeId& nodeId, uint32_t ip);
    void handleConnectMessage(const ConnectMessage& message);

    void fixFingers();

    void stabilise();

    void workThread();

    uint32_t findSuccessor(const NodeId& hash);
    uint32_t findSuccessor(const NodeId& nodeToQuery, const NodeId& hash);

    struct Neighbours
    {
      NodeId successor;
      NodeId predecessor;
      bool hasPredecessor;
    };
    uint32_t getNeighbours(const NodeId& nodeToQuery);

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

};

} // namespace chord

#endif // CHORD_NODE_H_

