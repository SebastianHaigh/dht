#include <arpa/inet.h>
#include <iostream>
#include <sys/socket.h>
#include <memory>
#include <unistd.h>
#include <fcntl.h>

#include "Acceptor.h"
#include "ClientManager.h"

namespace odd::io::tcp {

Acceptor::Acceptor(std::string ipAddress,
                               uint16_t portNumber,
                               ClientManager* clientManager)
  : m_fd(socket(AF_INET, SOCK_STREAM, 0)),
    m_clientManager(clientManager),
    m_running(false)
{
  m_address.sin_family = AF_INET;
  m_address.sin_port = htons(portNumber);
  inet_pton(AF_INET, ipAddress.c_str(), &m_address.sin_addr);
}

Acceptor::Acceptor(uint32_t ipAddress,
                               uint16_t portNumber,
                               ClientManager* clientManager)
  : m_fd(socket(AF_INET, SOCK_STREAM, 0)),
    m_clientManager(clientManager),
    m_running(false)
{
  m_address.sin_family = AF_INET;
  m_address.sin_port = htons(portNumber);
  m_address.sin_addr.s_addr = htonl(ipAddress);
}

Acceptor::~Acceptor()
{
  stop();
}

void Acceptor::start()
{
  m_running = true;

  if (not startListening())
  {
    return;
  }

  m_clientAcceptorThread = std::thread{ &Acceptor::clientAcceptorThreadFn, this };
}

bool Acceptor::startListening()
{
  // TODO (haigh) What happens if this function fails?
  if (m_fd < 0)
  {
    std::cerr << "Could not create client acceptor socket" << std::endl;
    return false;
  }

  int opt = 1;
  if (setsockopt(m_fd, SOL_SOCKET, SO_REUSEADDR, (char*) &opt, sizeof(opt)) == -1 ||
      setsockopt(m_fd, SOL_SOCKET, SO_REUSEPORT, (char*) &opt, sizeof(opt)) == -1)
  {
    std::cerr << "Could not set socket options" << std::endl;
    return false;
  }

  // Bind socket to IP and port
  if (bind(m_fd, (struct sockaddr*) &m_address, sizeof(m_address)) == -1)
  {
    std::cerr << "Could not bind to IP/port" << std::endl;
    return false;
  }

  // Mark socket for listening in
  if (listen(m_fd, SOMAXCONN) == -1)
  {
    std::cerr << "Could not listen" << std::endl;
    return false;
  }

  return true;
}

void Acceptor::stop()
{
  if (m_running)
  {
    m_running = false;
    m_clientAcceptorThread.join();
  }
}

void Acceptor::clientAcceptorThreadFn()
{
  fd_set listenSet;

  while(m_running)
  {
    std::cout << "Waiting for client" << std::endl;
    FD_ZERO(&listenSet);
    FD_SET(m_fd, &listenSet);

    // Accept a call
    sockaddr_in client;
    socklen_t clientSize = sizeof(client);

    timeval timeout;
    timeout.tv_sec = 1;

    int socketCount = select(m_fd + 1, &listenSet, nullptr, nullptr, &timeout);

    if (socketCount <= 0 || !FD_ISSET(m_fd, &listenSet))
    {
      continue;
    }

    int clientFD = accept(m_fd, (struct sockaddr*) &client, &clientSize);

    if (clientFD == -1)
    {
      // TODO (haigh) This is a hack to get around the fact that accept() is blocking
      // and we want to be able to stop the thread.  We should probably use select()
      // or something similar to make this non-blocking.
      if (errno == EAGAIN || errno == EWOULDBLOCK)
      {
        continue;
      }
      else
      {
        std::cerr << "Could not accept client" << std::endl;
        return;
      }
    }

    m_clientManager->processNewClient(std::make_unique<ClientRecord>(clientFD, client, clientSize));
  }

  std::cout << "Acceptor thread exiting" << std::endl;
}

} // namespace odd::io::tcp

