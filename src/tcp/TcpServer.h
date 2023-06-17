#ifndef TCP_SERVER_H_
#define TCP_SERVER_H_

#include "TcpAcceptor.h"
#include "TcpClientManager.h"
#include <functional>
#include <atomic>

class TcpServer_I 
{
  public:
    virtual ~TcpServer_I() {}
    virtual void start() = 0;
    virtual void stop() = 0;
    virtual void subscribeToAll(std::function<void(int, std::string)> callback) = 0;
    virtual void broadcast(const std::string& message) = 0;
    virtual void multicast(const std::string& message, std::vector<int> fds) = 0;
    virtual void unicast(const std::string& message, int fd) = 0;
};

class TcpServer : public TcpServer_I
{
  public:
    TcpServer(std::string ipAddress, uint16_t portNumber);
    ~TcpServer() override;

    void start() override;
    void stop() override;

    void subscribeToAll(std::function<void(int, std::string)> callback) override;
    void broadcast(const std::string& message) override;
    void multicast(const std::string& message, std::vector<int> fds) override;
    void unicast(const std::string& message, int fd) override;

  private:
    void threadFunction();
    TcpClientManager m_tcpClientManager;
    TcpClientAcceptor m_tcpClientAcceptor;
    std::vector<std::function<void(int, std::string)>> m_subscribers;
    std::thread m_thread;
    std::atomic<bool> m_running;
};

class LittleTestTcpServer
{
  public:
    LittleTestTcpServer(std::string ipAddress, uint16_t portNumber);

  private:
    TcpClientManager m_tcpClientManager;
    TcpClientAcceptor m_tcpClientAcceptor;

};


#endif // TCP_SERVER_H_

