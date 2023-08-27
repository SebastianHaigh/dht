#ifndef IO_TCP_CLIENT_H_
#define IO_TCP_CLIENT_H_

#include <cstdint>
#include <functional>
#include <netinet/in.h>
#include <sys/socket.h>
#include <thread>
#include <atomic>

#include "Types.h"

/*
 * Client allows a node to connect to other nodes. Each node has a TcpServer as well as mutliple
 * of these Clients. Each client connects the server in a single remote node.
 */

namespace odd::io::tcp {

class Client_I
{
  public:
    using onReceiveCallback = std::function<void()>;

    virtual ~Client_I() = default;

    virtual void start() = 0;
    virtual void stop() = 0;

    virtual void connectToServer() = 0;
    virtual void disconnect() = 0;
    virtual void send(const uint8_t* data, std::size_t size) = 0;
    virtual void setOnReceiveCallback(onReceiveCallback callback) = 0;
};

class Client : public Client_I
{
  public:
    Client() = default;
    Client(const IpAddressString& ipAddress, const PortNumber& port);
    Client(const IpAddressV4& ipAddress, const PortNumber& port);
    ~Client();

    void start() override;
    void stop() override;

    void connectToServer() override;
    void disconnect() override;
    void send(const uint8_t* data, std::size_t size) override;
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

} // namespace odd::io::tcp

#endif // IO_TCP_CLIENT_H_

