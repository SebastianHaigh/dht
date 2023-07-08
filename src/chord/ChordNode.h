#ifndef CHORD_NODE_H_
#define CHORD_NODE_H_

#include "../tcp/TcpServer.h"
#include "../tcp/TcpClient.h"
#include "../async/ThreadPool.h"

#include "NodeId.h"
#include "FingerTable.h"

namespace chord {

static constexpr uint8_t NULL_NODE_ID[20] = { 0 };

// The inter-node request is the message format used to communicate between chord nodes.
// The message format is as follows:
// 1. The type of request (JOIN, LEAVE, FIND_SUCCESSOR, FIND_PREDECESSOR, CLOSEST_PRECEDING_FINGER)
// 2. The source node id
// 3. The destination node id
// 4. The source node ip address
// 5. The source node port
// The message is sent over TCP as an array of bytes, with each field being converted to network byte order.
// The message is received over TCP as an array of bytes, with each field being converted to host byte order.
// The message is then converted to the InterNodeRequest struct.
// The array of bytes will be:
// 1. The type of request (2 bytes)
// 2. The source node id (20 bytes)
// 3. The destination node id (20 bytes)
// 4. The source node ip address (4 bytes)
// 5. The source node port (2 bytes)
// The total size of the message is 48 bytes.
struct InterNodeRequest
{
  enum class Type : uint16_t
  {
    // Request Types, values are in the range 0x0000 - 0x00FF
    JOIN_REQUEST = 0x0001,
    LEAVE_REQUEST = 0x0002,
    FIND_SUCCESSOR_REQUEST = 0x0003,
    FIND_PREDECESSOR_REQUEST = 0x0004,

    // Response Types, values are in the range 0x0100 - 0x01FF
    JOIN_RESPONSE = 0x0101,
    LEAVE_RESPONSE = 0x0102,
    FIND_SUCCESSOR_RESPONSE = 0x0103,
    FIND_PREDECESSOR_RESPONSE = 0x0104,
  };

  InterNodeRequest(Type type,
                   const NodeId& sourceId,
                   const NodeId& destinationId,
                   uint32_t sourceIpAddress,
                   uint16_t sourcePort)
    : m_type(type),
      m_sourceId(sourceId),
      m_destinationId(destinationId),
      m_sourceIpAddress(sourceIpAddress),
      m_sourcePort(sourcePort)
  {
  }

  InterNodeRequest(uint8_t *buffer, size_t size)
  {
    if (size != 48)
    {
      throw std::runtime_error("Invalid size for InterNodeRequest");
    }

    m_type = static_cast<Type>(ntohs(*reinterpret_cast<uint16_t*>(buffer)));
    buffer += 2;
    memcpy(m_sourceId.m_id, buffer, 20);
    buffer += 20;
    memcpy(m_destinationId.m_id, buffer, 20);
    buffer += 20;
    m_sourceIpAddress = ntohl(*reinterpret_cast<uint32_t*>(buffer));
    buffer += 4;
    m_sourcePort = ntohs(*reinterpret_cast<uint16_t*>(buffer));
  }

  void toBuffer(uint8_t *buffer, size_t size)
  {
    if (size != 48)
    {
      throw std::runtime_error("Invalid size for InterNodeRequest");
    }

    *reinterpret_cast<uint16_t*>(buffer) = htons(static_cast<uint16_t>(m_type));
    buffer += 2;
    memcpy(buffer, m_sourceId.m_id, 20);
    buffer += 20;
    memcpy(buffer, m_destinationId.m_id, 20);
    buffer += 20;
    *reinterpret_cast<uint32_t*>(buffer) = htonl(m_sourceIpAddress);
    buffer += 4;
    *reinterpret_cast<uint16_t*>(buffer) = htons(m_sourcePort);
  }

  Type m_type;
  NodeId m_sourceId;
  NodeId m_destinationId;
  uint32_t m_sourceIpAddress;
  uint16_t m_sourcePort;
};

using IpAddress = std::string;

class ChordNode
{
  public:
    ChordNode(const std::string& ip, uint16_t port);

    void join(const std::string &knownNodeIpAddress);
    NodeId& getId();

    const NodeId& getPredecessorId();
    const NodeId& getSuccessorId();

    std::future<NodeId> findSuccessor(const NodeId& id);
    NodeId findPredecessor(const NodeId& id);
    const NodeId& closestPrecedingFinger(const NodeId& id);

  private:

    NodeId doFindSuccessor(const NodeId& id);
    void doFindPredecessor(const NodeId& id);

    void initialiseFingerTable();
    void updateOthers();
    void updateFingerTable(const ChordNode& node, uint16_t i);

    void processReceivedMessage(const InterNodeRequest& request);

    void sendFindSuccessorRequest(const NodeId& id);
    void sendFindPredecessorRequest(const NodeId& id);

    static NodeId createNodeId(const std::string& ipAddress);
    static uint32_t convertIpAddressToInteger(const std::string& ipAddress);

    NodeId m_id;
    NodeId m_predecessor;
    NodeId m_successor;
    FingerTable m_fingerTable;
    const uint32_t m_ipAddress;
    const uint16_t m_port;

    ThreadPool m_threadPool;

    tcp::TcpServer m_tcpServer;

    struct NodeConnection
    {
      NodeConnection(const NodeId& id, const std::string& ipAddress, uint16_t port)
        : m_id(id),
          m_tcpClient(std::make_unique<tcp::TcpClient>(ipAddress, port))
      {
      }
      NodeId m_id;
      std::unique_ptr<tcp::TcpClient_I> m_tcpClient;
    };

    std::vector<NodeConnection> m_nodeConnections;
};

} // namespace chord

#endif // CHORD_NODE_H_

