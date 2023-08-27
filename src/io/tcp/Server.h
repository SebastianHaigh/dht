#ifndef IO_TCP_SERVER_H_
#define IO_TCP_SERVER_H_

#include "Acceptor.h"
#include "ClientManager.h"
#include <functional>
#include <atomic>

namespace odd::io::tcp {

using OnReceiveCallback = std::function<void(uint8_t*, std::size_t)>;

class Server_I
{
  public:
    virtual ~Server_I() = default;
    virtual void start() = 0;
    virtual void stop() = 0;
    virtual void subscribeToAll(OnReceiveCallback callback) = 0;
    virtual void broadcast(const std::string& message) = 0;
    virtual void multicast(const std::string& message, std::vector<int> fds) = 0;
    virtual void unicast(const std::string& message, int fd) = 0;
};

class Server : public Server_I
{
  public:
    Server(std::string ipAddress, uint16_t portNumber);
    Server(uint32_t ipAddress, uint16_t portNumber);
    ~Server() override;

    void start() override;
    void stop() override;

    void subscribeToAll(OnReceiveCallback callback) override;
    void broadcast(const std::string& message) override;
    void multicast(const std::string& message, std::vector<int> fds) override;
    void unicast(const std::string& message, int fd) override;

  private:
    void threadFunction();
    ClientManager m_clientManager;
    ClientAcceptor m_acceptor;
    std::vector<OnReceiveCallback> m_subscribers;
    std::thread m_thread;
    std::atomic<bool> m_running;
};

} // namespace odd::io::tcp

#endif // IO_TCP_SERVER_H_

