#include "TcpServer.h"
#include <cstring>
#include <iostream>

namespace odd::tcp {

TcpServer::TcpServer(std::string ipAddress, uint16_t portNumber)
  : m_tcpClientAcceptor{ipAddress, portNumber, &m_tcpClientManager},
    m_running(false)
{
}

TcpServer::TcpServer(uint32_t ipAddress, uint16_t portNumber)
  : m_tcpClientAcceptor{ipAddress, portNumber, &m_tcpClientManager},
    m_running(false)
{
}

TcpServer::~TcpServer()
{
  stop();
}

void TcpServer::start()
{
  try
  {
    m_running = true;
    m_thread = std::thread(&TcpServer::threadFunction, this);
    m_tcpClientAcceptor.start();
  }
  catch (const std::exception& e)
  {
    std::cout << "TcpServer could not be started: " << e.what() << std::endl;
  }
}

void TcpServer::stop()
{
  if (m_running)
  {
    m_running = false;
    m_thread.join();
    m_tcpClientAcceptor.stop();
  }
}

void TcpServer::subscribeToAll(OnReceiveCallback callback)
{
  m_subscribers.push_back(callback);
}

void TcpServer::broadcast(const std::string& message)
{
  for (const auto& fd : m_tcpClientManager.getAllClientFds())
  {
    unicast(message, fd);
  }
}

void TcpServer::multicast(const std::string& message, std::vector<int> fds)
{
  for (const auto& fd : fds)
  {
    unicast(message, fd);
  }
}

void TcpServer::unicast(const std::string& message, int fd)
{
  send(fd, message.c_str(), message.size() + 1, 0);
}

void TcpServer::threadFunction()
{
  fd_set master;

  while (m_running)
  {
    FD_ZERO(&master);

    auto allClientFds = m_tcpClientManager.getAllClientFds();

    for (const auto& fd : allClientFds)
    { 
      FD_SET(fd, &master);
    };
    
    timeval timeout;
    timeout.tv_sec = 1;

    int socketCount = select(40, &master, nullptr, nullptr, &timeout);

    if (socketCount <= 0)
    {
      continue;
    }

    for (const auto& client : allClientFds)
    {
      char messageBuffer[4096];
      std::memset(messageBuffer, 0, 4096);

      if (FD_ISSET(client, &master))
      {
        size_t bytesIn = recv(client, &messageBuffer, 4096, 0);

        if (bytesIn <= 0)
        {
          m_tcpClientManager.processClientDisconnecion(client);
        }
        else
        {
          for (const auto& subscriber : m_subscribers)
          {
            auto* messageData = new uint8_t[bytesIn];
            memcpy(messageData, messageBuffer, bytesIn);
            subscriber(messageData, bytesIn);
          }
        }
      }

    }
  }


}

} // namespace odd::tcp

