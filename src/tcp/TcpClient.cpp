#include "TcpClient.h"

#include <arpa/inet.h>
#include <unistd.h>


namespace tcp {

TcpClient::TcpClient(const IpAddressString& ipAddress, const PortNumber& port)
  : m_socket(-1), 
    m_ip(inet_addr(ipAddress.c_str())), 
    m_port(port),
    m_onReceiveCallback(nullptr)
{
}

TcpClient::~TcpClient()
{
  disconnect();
  m_receiveThread.join();
}

void TcpClient::connectToServer()
{
  m_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (m_socket < 0)
  {
  }

  sockaddr_in serverAddress;
  serverAddress.sin_family = AF_INET;
  serverAddress.sin_addr.s_addr = m_ip;
  serverAddress.sin_port = htons(m_port);

  if (connect(m_socket, (sockaddr*)&serverAddress, sizeof(serverAddress)) < 0)
  {
  }
}

void TcpClient::disconnect()
{
  if (m_socket >= 0)
  {
    close(m_socket);
    m_socket = -1;
  }
}

void TcpClient::send(const char* data, int size)
{
  if (m_socket < 0)
  {
  }

  if (::send(m_socket, data, size, 0) < 0)
  {
  }
}

void TcpClient::setOnReceiveCallback(onReceiveCallback callback)
{
  m_onReceiveCallback = callback;
}

void TcpClient::receiveThreadFunction()
{
  while (true)
  {
    char buffer[1024];
    int receivedBytes = recv(m_socket, buffer, sizeof(buffer), 0);
    if (receivedBytes < 0)
    {
      
    }
    else if (receivedBytes == 0)
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
}

} // namespace tcp

