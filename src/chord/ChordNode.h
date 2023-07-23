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

class ChordNode
{
  public:
    ChordNode(const std::string& ip, uint16_t port, const ConnectionManagerFactory& factory);

    void create();
    void join(const std::string &knownNodeIpAddress);
    NodeId& getId();

    const std::optional<NodeId>& getPredecessorId();
    const NodeId& getSuccessorId();

    const NodeId& closestPrecedingFinger(const NodeId& id);
    void receive(uint8_t* message, std::size_t messageLength);

  private:

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

    void stabilise();

    std::future<NodeId> findSuccessor(const NodeId& hash);
    std::future<NodeId> findSuccessor(const NodeId& nodeToQuery, const NodeId& hash);

    struct Neighbours
    {
      NodeId successor;
      NodeId predecessor;
      bool hasPredecessor;
    };
    std::future<Neighbours> getNeighbours(const NodeId& nodeToQuery);

    void notify(const NodeId& nodeId);

    static uint32_t convertIpAddressToInteger(const std::string& ipAddress);

    uint32_t getNextAvailableRequestId();

    const uint32_t m_ipAddress;
    NodeId m_id;

    std::optional<NodeId> m_predecessor;
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
    std::unordered_map<uint32_t, std::promise<Neighbours>> m_getNeighboursPromises;
    std::promise<NodeId> m_joinPromise;
};

} // namespace chord

#endif // CHORD_NODE_H_

