#ifndef TCP_ACCEPTOR_H_
#define TCP_ACCEPTOR_H_

#include <cstdint>
#include <netinet/in.h> // sockaddr_in
#include <thread>
#include <atomic>

#include "TcpClientManager.h"

namespace odd {

class TcpClientAcceptor
{
  public:
    TcpClientAcceptor(std::string ipAddress, uint16_t portNumber, TcpClientManager* clientManager);
    TcpClientAcceptor(uint32_t ipAddress, uint16_t portNumber, TcpClientManager* clientManager);
    ~TcpClientAcceptor();   

    void start();
    void stop();

  private:
    bool startListening();
    void clientAcceptorThreadFn();

    int m_fd;
    sockaddr_in m_address;
    std::thread m_clientAcceptorThread;
    TcpClientManager* m_clientManager;
    std::atomic<bool> m_running;
};

} // namespace odd

#endif // TCP_ACCEPTOR_H_
