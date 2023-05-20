#ifndef TCP_CLIENT_H_
#define TCP_CLIENT_H_

#include <functional>
#include <thread>

#include "TcpTypes.h"

namespace tcp {

class TcpClient_I
{
  public:
    using onReceiveCallback = std::function<void()>;

    virtual ~TcpClient_I() = default;
    virtual void connectToServer() = 0;
    virtual void disconnect() = 0;
    virtual void send(const char* data, int size) = 0;
    virtual void setOnReceiveCallback(onReceiveCallback callback) = 0;
};

class TcpClient : public TcpClient_I
{
  public:
    TcpClient(const IpAddressString& ipAddress, const PortNumber& port);
    ~TcpClient();

    void connectToServer() override;
    void disconnect() override;
    void send(const char* data, int size) override;
    void setOnReceiveCallback(onReceiveCallback callback) override;

  private:
    void receiveThreadFunction();

    int m_socket;
    const IpAddressV4 m_ip;
    PortNumber m_port;
    onReceiveCallback m_onReceiveCallback;
    std::thread m_receiveThread;

};

} // namespace tcp

#endif // TCP_CLIENT_H_

