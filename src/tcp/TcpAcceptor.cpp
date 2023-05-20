#include <arpa/inet.h>
#include <iostream>
#include <sys/socket.h>
#include <memory>

#include "TcpAcceptor.h"
#include "TcpClientManager.h"

TcpClientAcceptor::TcpClientAcceptor(std::string ipAddress,
                                     uint16_t portNumber,
                                     TcpClientManager* clientManager)
  : m_fd(socket(AF_INET, SOCK_STREAM, 0)),
    m_clientManager(clientManager)
{
  m_address.sin_family = AF_INET;
  m_address.sin_port = htons(portNumber);
  inet_pton(AF_INET, ipAddress.c_str(), &m_address.sin_addr);
}

TcpClientAcceptor::~TcpClientAcceptor()
{
  m_clientAcceptorThread.join();
}

void TcpClientAcceptor::start()
{
  m_clientAcceptorThread = std::thread{ &TcpClientAcceptor::clientAcceptorThreadFn, this };
}

bool TcpClientAcceptor::startListening()
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

void TcpClientAcceptor::stop()
{
}

void TcpClientAcceptor::clientAcceptorThreadFn()
{
  if (not startListening())
  {
    return;
  }

  while(true)
  {
    // Accept a call
    sockaddr_in client;
    socklen_t clientSize = sizeof(client);

    int clientFD = accept(m_fd, (struct sockaddr*) &client, &clientSize);

    m_clientManager->processNewClient(std::make_unique<TcpClient>(clientFD, client, clientSize));
  }
}

