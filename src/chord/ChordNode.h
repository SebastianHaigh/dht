#ifndef CHORD_NODE_H_
#define CHORD_NODE_H_

#include "../tcp/TcpServer.h"
#include "../tcp/TcpClient.h"
#include "../async/ThreadPool.h"
#include "../comms/Comms.h"

#include "NodeId.h"
#include "FingerTable.h"

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

    std::future<NodeId> findSuccessor(const NodeId& id);
    NodeId findPredecessor(const NodeId& id);
    const NodeId& closestPrecedingFinger(const NodeId& id);

  private:

    NodeId doFindSuccessor(const NodeId& id);
    void doFindPredecessor(const NodeId& id);

    void initialiseFingerTable();
    void updateOthers();
    void updateFingerTable(const ChordNode& node, uint16_t i);

    void processReceivedMessage(const Message& request);

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

