#ifndef CONNECTION_MANAGER_H_
#define CONNECTION_MANAGER_H_

#include "../tcp/TcpServer.h"
#include "../tcp/TcpClient.h"
#include "../comms/Comms.h"

#include "NodeId.h"

namespace chord {

/*
 * The ConnectionManager is used for managing the external network connection that this not has to
 * other nodes. The concrete production version will contain a tcp server and multip tcp clients.
 *
 * Eventually this sill be derived from a virtual interface that can be mocked using the network
 * simulation code.
 */
class ConnectionManager
{
  public:
    ConnectionManager(const NodeId& nodeId, uint32_t ip, uint16_t port);

    bool send(const NodeId& nodeId, const Message& message);

    void registerReceiveHandler(tcp::OnReceiveCallback callback);

    void insert(const NodeId& id, uint32_t ipAddress, uint16_t port);

    void remove(const NodeId& id);

  private:

    struct NodeConnection
    {
      NodeConnection(const NodeId& id, uint32_t ipAddress, uint16_t port)
        : m_id(id),
          m_tcpClient(std::make_unique<tcp::TcpClient>(ipAddress, port))
      {
      }
      NodeId m_id;
      std::unique_ptr<tcp::TcpClient_I> m_tcpClient;
    };

    std::vector<NodeConnection>::iterator getNodeConnection(const NodeId& nodeId);

    std::size_t getClientIndex(const NodeId& nodeId);

    tcp::TcpServer m_server;
    std::vector<NodeConnection> m_nodeConnections;

    NodeId m_localNodeId;
    const uint32_t m_localIpAddress;
    const uint16_t m_localPort;
};

} // namespace chord

#endif // CONNECTION_MANAGER_H_

