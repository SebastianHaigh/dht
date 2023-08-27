#include "Network.h"

#include <arpa/inet.h>
#include <cstdint>
#include <iostream>
#include <memory>
#include <utility>

namespace odd::io::simulation {

Network::Network()
  : m_nextNodeId(0)
{
}

void Network::run()
{
}

Node& Network::addNode(const std::string& ipAddress)
{
  uint32_t ip_int{0};
  inet_pton(AF_INET, ipAddress.c_str(), &ip_int);
  return addNode(ip_int);
}

Node& Network::addNode(const std::string& ipAddress, Node::ReceiveHandler receiveHandler)
{
  uint32_t ip_int{0};
  inet_pton(AF_INET, ipAddress.c_str(), &ip_int);
  return addNode(ip_int, std::move(receiveHandler));
}

Node& Network::addNode(uint32_t ipAddress)
{
  m_nodeIds.push_back(m_nextNodeId++);

  auto onSendCallback = [this] (uint32_t source,
                                uint32_t dest,
                                uint8_t* message,
                                size_t messageLength)
                        {
                          sendMessage(source,
                                      dest,
                                      message,
                                      messageLength);
                        };

  auto node = std::make_unique<Node>(m_nodeIds.back(),
                                              ipAddress,
                                              onSendCallback);

  m_nodes.push_back(std::move(node));

  m_nodeIdLookup.emplace(ipAddress, m_nodeIds.back());

  if (m_nodes.size() == 1) return *m_nodes.back();

  return *m_nodes.back();

}

Node& Network::addNode(uint32_t ipAddress, Node::ReceiveHandler receiveHandler)
{
  m_nodeIds.push_back(m_nextNodeId++);

  auto onSendCallback = [this] (uint32_t source,
                                uint32_t dest,
                                uint8_t* message,
                                size_t messageLength)
                        {
                          sendMessage(source,
                                      dest,
                                      message,
                                      messageLength);
                        };

  auto node = std::make_unique<Node>(m_nodeIds.back(),
                                              ipAddress,
                                              onSendCallback,
                                              std::move(receiveHandler));

  m_nodes.push_back(std::move(node));

  // TODO (haigh) do I need some mechanism to remove these if a node leaves?
  m_nodeIdLookup.emplace(ipAddress, m_nodeIds.back());

  if (m_nodes.size() == 1) return *m_nodes.back();

  return *m_nodes.back();
}

void Network::sendMessage(uint32_t sourceIpAddress,
                                   uint32_t destinationIpAddress,
                                   uint8_t* message,
                                   size_t messageLength)
{
  // Go through each of the nodes that we have and find the destination Ip Address
  auto nodeIp_p = m_nodeIdLookup.find(destinationIpAddress);

  if (nodeIp_p == m_nodeIdLookup.end()) return;

  for (auto& node : m_nodes)
  {
    if (node->nodeId() == nodeIp_p->second)
    {
      node->receiveMessage(sourceIpAddress, message, messageLength);
    }
  }
}

} // namespace odd::io::simulation

