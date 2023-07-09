#include "TcpClient.h"
#include "TcpTypes.h"

#include <cstdint>
#include <iostream>

#include <arpa/inet.h>
#include <unistd.h>

namespace tcp {

TcpClient::TcpClient(const IpAddressString& ipAddress, const PortNumber& port)
  : m_fd(-1),
    m_onReceiveCallback(nullptr),
    m_running(false)
{
  // TODO (haigh) add a logger to do this instead
  std::cout << "creating socket address" << std::endl;
  m_socketAddress.sin_family = AF_INET;
  m_socketAddress.sin_port = htons(port);
  inet_pton(AF_INET, ipAddress.c_str(), &m_socketAddress.sin_addr);

  m_socketAddressLength = sizeof(m_socketAddress);
}

TcpClient::TcpClient(const IpAddressV4& ipAddress, const PortNumber& port)
  : m_fd(-1),
    m_onReceiveCallback(nullptr),
    m_running(false)
{
  // TODO (haigh) add a logger to do this instead
  std::cout << "creating socket address" << std::endl;
  m_socketAddress.sin_family = AF_INET;
  m_socketAddress.sin_port = htons(port);
  m_socketAddress.sin_addr.s_addr = htonl(ipAddress);

  m_socketAddressLength = sizeof(m_socketAddress);
}

TcpClient::~TcpClient()
{
  stop();
}

void TcpClient::start()
{
  m_running = true;
  connectToServer();
  m_receiveThread = std::thread{&TcpClient::receiveThreadFunction, this};
}

void TcpClient::stop()
{
  if (m_running)
  {
    m_running = false;
    m_receiveThread.join();
    disconnect();
  }
}

void TcpClient::connectToServer()
{
  m_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (m_fd < 0)
  {
    std::cout << "Error creating socket" << std::endl;
  }

  while (connect(m_fd, (struct sockaddr*) &m_socketAddress, m_socketAddressLength) < 0)
  {
    std::cout << "Error connecting to server" << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
}

void TcpClient::disconnect()
{
  if (m_fd >= 0)
  {
    close(m_fd);
    m_fd = -1;
  }
}

void TcpClient::send(const uint8_t* data, std::size_t size)
{
  if (!m_running || m_fd < 0)
  {
    return;
  }

  if (::send(m_fd, data, size, 0) < 0)
  {
    std::cout << "Error sending data" << std::endl;
  }
}

void TcpClient::setOnReceiveCallback(onReceiveCallback callback)
{
  m_onReceiveCallback = callback;
}

void TcpClient::receiveThreadFunction()
{
  fd_set toReadFdSet;

  while (m_running)
  {
    std::cout << "receiving" << std::endl;
    FD_ZERO(&toReadFdSet);
    FD_SET(m_fd, &toReadFdSet);

    timeval timeout;
    timeout.tv_sec = 1;

    int socketCount = select(m_fd + 1, &toReadFdSet, nullptr, nullptr, &timeout);

    if (socketCount <= 0 || !FD_ISSET(m_fd, &toReadFdSet))
    {
      std::this_thread::sleep_for(std::chrono::seconds(1));
      continue;
    }

    char buffer[1024];

    int receivedBytes = recv(m_fd, buffer, sizeof(buffer), 0);
    if (receivedBytes <= 0)
    {
      // If this happens, the server has closed the connection.
      // We should handle this case.
      break;
    }
    else
    {
      if (m_onReceiveCallback)
      {
        m_onReceiveCallback();
      }
    }
  }

  std::cout << "receive thread exiting" << std::endl;
}

} // namespace tcp

