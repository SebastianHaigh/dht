#ifndef IO_TCP_ACCEPTOR_H_
#define IO_TCP_ACCEPTOR_H_

#include <cstdint>
#include <netinet/in.h> // sockaddr_in
#include <thread>
#include <atomic>

#include "ClientManager.h"

namespace odd::io::tcp {

class Acceptor
{
  public:
    Acceptor(std::string ipAddress, uint16_t portNumber, ClientManager* clientManager);
    Acceptor(uint32_t ipAddress, uint16_t portNumber, ClientManager* clientManager);
    ~Acceptor();

    void start();
    void stop();

  private:
    bool startListening();
    void clientAcceptorThreadFn();

    int m_fd;
    sockaddr_in m_address;
    std::thread m_clientAcceptorThread;
    ClientManager* m_clientManager;
    std::atomic<bool> m_running;
};

} // namespace odd::io::tcp

#endif // IO_TCP_ACCEPTOR_H_
