#include "ClientManager.h"
#include <arpa/inet.h>
#include <netdb.h>
#include <iostream>
#include <unistd.h>

namespace odd::io::tcp {

ClientRecord::ClientRecord(int fd, sockaddr_in socketAddress, socklen_t addressLength)
  : m_fd(fd),
    m_socketAddress(socketAddress),
    m_socketAddressLength(addressLength)
{
  char address[NI_MAXHOST];
  inet_ntop(AF_INET, &m_socketAddress.sin_addr, address, NI_MAXHOST);
  m_name = std::string{address} + ":" + std::to_string(ntohs(m_socketAddress.sin_port));
}

const std::string ClientRecord::socketName()
{
  return m_name;
}

const std::string& ClientRecord::socketName() const
{
  return m_name;
}

void ClientManager::processNewClient(std::unique_ptr<ClientRecord> client)
{
  std::lock_guard<std::mutex> lock(m_mutex);

  std::cout << "New client connected: " << client->socketName() << std::endl;
  m_clients.emplace(client->m_fd, std::move(client));

  m_conditionVariable.notify_one();
}

void ClientManager::processClientDisconnecion(int fd)
{
  std::lock_guard<std::mutex> lock(m_mutex);

  auto data = m_clients.find(fd);

  if (data == m_clients.end())
  {
    m_conditionVariable.notify_one();
    return;
  }

  m_clients.erase(data);
  close(fd);

  m_conditionVariable.notify_one();
}

bool ClientManager::hasClient(int fd)
{
  std::lock_guard<std::mutex> lock(m_mutex);

  auto data = m_clients.find(fd);

  if (data != m_clients.end())
  {
    return true;
  }

  return false;
}

std::string ClientManager::getClientName(int fd)
{
  std::lock_guard<std::mutex> lock(m_mutex);

  auto data = m_clients.find(fd);

  if (data != m_clients.end())
  {
    return data->second->socketName();
  }

  return "Unknown client";
}

std::vector<int> ClientManager::getAllClientFds()
{
  std::lock_guard<std::mutex> lock(m_mutex);

  std::vector<int> result;

  for (const auto& data : m_clients)
  {
    result.push_back(data.first);
  }

  return result;
}

} // namespace odd::io::tcp

