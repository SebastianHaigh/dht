#ifndef CHORD_NODE_H_
#define CHORD_NODE_H_

#include "../comms/Comms.h"

#include "ChordMessaging.h"
#include "NodeId.h"
#include "FingerTable.h"
#include "ConnectionManager.h"

namespace chord {

static constexpr uint8_t NULL_NODE_ID[20] = { 0 };

using IpAddress = std::string;


class ChordNode
{
  public:
    ChordNode(const std::string& ip, uint16_t port);

    void join(const std::string &knownNodeIpAddress);
    NodeId& getId();

    const NodeId& getPredecessorId();
    const NodeId& getSuccessorId();

    const NodeId& closestPrecedingFinger(const NodeId& id);

  private:

    void doFindSuccessor(const FindSuccessorMessage& message);

    void initialiseFingerTable();
    void updateFingerTable(const ChordNode& node, uint16_t i);

    void handleReceivedMessage(const EncodedMessage& encoded);

    static NodeId createNodeId(const std::string& ipAddress);
    static uint32_t convertIpAddressToInteger(const std::string& ipAddress);

    uint32_t getNextAvailableRequestId();

    NodeId m_id;
    NodeId m_predecessor;
    NodeId m_successor;
    FingerTable m_fingerTable;
    const uint32_t m_ipAddress;
    const uint16_t m_port;

    ConnectionManager m_connectionManager;

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
};

} // namespace chord

#endif // CHORD_NODE_H_

