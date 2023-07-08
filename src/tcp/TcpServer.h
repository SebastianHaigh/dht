#ifndef TCP_SERVER_H_
#define TCP_SERVER_H_

#include "TcpAcceptor.h"
#include "TcpClientManager.h"
#include <functional>
#include <atomic>

namespace tcp {

using OnReceiveCallback = std::function<void(uint8_t*, std::size_t)>;

class TcpServer_I 
{
  public:
    virtual ~TcpServer_I() {}
    virtual void start() = 0;
    virtual void stop() = 0;
    virtual void subscribeToAll(OnReceiveCallback callback) = 0;
    virtual void broadcast(const std::string& message) = 0;
    virtual void multicast(const std::string& message, std::vector<int> fds) = 0;
    virtual void unicast(const std::string& message, int fd) = 0;
};

class TcpServer : public TcpServer_I
{
  public:
    TcpServer(std::string ipAddress, uint16_t portNumber);
    TcpServer(uint32_t ipAddress, uint16_t portNumber);
    ~TcpServer() override;

    void start() override;
    void stop() override;

    void subscribeToAll(OnReceiveCallback callback) override;
    void broadcast(const std::string& message) override;
    void multicast(const std::string& message, std::vector<int> fds) override;
    void unicast(const std::string& message, int fd) override;

  private:
    void threadFunction();
    TcpClientManager m_tcpClientManager;
    TcpClientAcceptor m_tcpClientAcceptor;
    std::vector<OnReceiveCallback> m_subscribers;
    std::thread m_thread;
    std::atomic<bool> m_running;
};

} // namespace tcp

#endif // TCP_SERVER_H_

