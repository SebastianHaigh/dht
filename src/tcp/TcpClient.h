#ifndef TCP_CLIENT_H_
#define TCP_CLIENT_H_

#include <cstdint>
#include <functional>
#include <netinet/in.h>
#include <sys/socket.h>
#include <thread>
#include <atomic>

#include "TcpTypes.h"

/*
 * TcpClient allows a node to connect to other nodes. Each node has a TcpServer as well as mutliple
 * of these TcpClients. Each client connects the server in a single remote node.
 */

namespace tcp {

class TcpClient_I
{
  public:
    using onReceiveCallback = std::function<void()>;

    virtual ~TcpClient_I() = default;

    virtual void start() = 0;
    virtual void stop() = 0;

    virtual void connectToServer() = 0;
    virtual void disconnect() = 0;
    virtual void send(const uint8_t* data, int size) = 0;
    virtual void setOnReceiveCallback(onReceiveCallback callback) = 0;
};

class TcpClient : public TcpClient_I
{
  public:
    TcpClient() = default;
    TcpClient(const IpAddressString& ipAddress, const PortNumber& port);
    ~TcpClient();

    void start() override;
    void stop() override;

    void connectToServer() override;
    void disconnect() override;
    void send(const uint8_t* data, int size) override;
    void setOnReceiveCallback(onReceiveCallback callback) override;

  private:
    void receiveThreadFunction();

    int m_fd;
    sockaddr_in m_socketAddress;
    socklen_t m_socketAddressLength;
    
    onReceiveCallback m_onReceiveCallback;
    std::thread m_receiveThread;
    std::atomic<bool> m_running;
};

} // namespace tcp

#endif // TCP_CLIENT_H_

