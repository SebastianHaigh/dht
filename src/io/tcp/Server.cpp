#include "Server.h"
#include <cstring>
#include <iostream>

namespace odd::io::tcp {

Server::Server(std::string ipAddress, uint16_t portNumber)
  : m_acceptor{ipAddress, portNumber, &m_clientManager},
    m_running(false)
{
}

Server::Server(uint32_t ipAddress, uint16_t portNumber)
  : m_acceptor{ipAddress, portNumber, &m_clientManager},
    m_running(false)
{
}

Server::~Server()
{
  stop();
}

void Server::start()
{
  try
  {
    m_running = true;
    m_thread = std::thread(&Server::threadFunction, this);
    m_acceptor.start();
  }
  catch (const std::exception& e)
  {
    std::cout << "Server could not be started: " << e.what() << std::endl;
  }
}

void Server::stop()
{
  if (m_running)
  {
    m_running = false;
    m_thread.join();
    m_acceptor.stop();
  }
}

void Server::subscribeToAll(OnReceiveCallback callback)
{
  m_subscribers.push_back(callback);
}

void Server::broadcast(const std::string& message)
{
  for (const auto& fd : m_clientManager.getAllClientFds())
  {
    unicast(message, fd);
  }
}

void Server::multicast(const std::string& message, std::vector<int> fds)
{
  for (const auto& fd : fds)
  {
    unicast(message, fd);
  }
}

void Server::unicast(const std::string& message, int fd)
{
  send(fd, message.c_str(), message.size() + 1, 0);
}

void Server::threadFunction()
{
  fd_set master;

  while (m_running)
  {
    FD_ZERO(&master);

    auto allClientFds = m_clientManager.getAllClientFds();

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
          m_clientManager.processClientDisconnecion(client);
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

} // namespace odd::io::tcp

