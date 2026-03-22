#ifndef CONNECTION_MANAGER_H_
#define CONNECTION_MANAGER_H_

#include <tcp/Server.h>
#include <tcp/Client.h>
#include <boost/asio/ip/address.hpp>
#include "../comms/Comms.h"

#include "NodeId.h"

namespace odd::chord {

class ConnectionManager_I
{
  public:
    virtual ~ConnectionManager_I() = default;

    virtual bool send(const NodeId& nodeId,
                      const Message& message) = 0;

    virtual bool broadcast(const Message& message) = 0;

    virtual void registerReceiveHandler(io::tcp::OnReceiveCallback callback) = 0;

    virtual void insert(const NodeId& id,
                        const boost::asio::ip::address& ipAddress,
                        uint16_t port) = 0;

    virtual void remove(const NodeId& id) = 0;

    virtual void stop() = 0;

    [[nodiscard]] virtual const boost::asio::ip::address& ip() const = 0;

    [[nodiscard]] virtual boost::asio::ip::address ip(const NodeId& nodeId) const = 0;
};

/*
 * The ConnectionManager is used for managing the external network connection that this not has to
 * other nodes. The concrete production version will contain a tcp server and multip tcp clients.
 */
class ConnectionManager : public ConnectionManager_I
{
  public:
    ConnectionManager(const NodeId& nodeId,
                      boost::asio::ip::address ip,
                      uint16_t port);

    bool send(const NodeId& nodeId, const Message& message) override;

    bool broadcast(const Message& message) override { return false; }

    void registerReceiveHandler(io::tcp::OnReceiveCallback callback) override;

    void insert(const NodeId& id,
                const boost::asio::ip::address& ipAddress,
                uint16_t port) override;

    void remove(const NodeId& id) override;

    void stop() override { };

    [[nodiscard]] const boost::asio::ip::address& ip() const override;

    [[nodiscard]] boost::asio::ip::address ip(const NodeId& nodeId) const override;

  private:

    struct NodeConnection
    {
      NodeConnection(const NodeId& id, const boost::asio::ip::address& ipAddress, uint16_t port)
        : m_id(id),
          m_tcpClient(std::make_unique<io::tcp::Client>(ipAddress, port)),
          m_ip(ipAddress)
      {
      }
      NodeId m_id;
      std::unique_ptr<io::tcp::Client_I> m_tcpClient;
      boost::asio::ip::address m_ip;
    };

    std::vector<NodeConnection>::iterator getNodeConnection(const NodeId& nodeId);
    std::vector<NodeConnection>::const_iterator getNodeConnection(const NodeId& nodeId) const;

    std::size_t getClientIndex(const NodeId& nodeId);

    io::tcp::Server m_server;
    std::vector<NodeConnection> m_nodeConnections;

    NodeId m_localNodeId;
    const boost::asio::ip::address m_localIpAddress;
    const uint16_t m_localPort;
};

} // namespace odd::chord

#endif // CONNECTION_MANAGER_H_

