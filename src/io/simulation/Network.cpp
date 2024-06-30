#include "Network.h"

#include <arpa/inet.h>
#include <cstdint>
#include <iostream>
#include <memory>
#include <utility>
#include "EncodedMessage.h"

namespace odd::io::simulation {

Network::Network()
  : m_nextNodeId(0)
{
}

void Network::run()
{
  if (m_receivedMessages.empty()) return;

  // Send all of the messages that were in this queue when the method was called
  auto toSend = m_receivedMessages.size();

  for (int i = 0; i < toSend; ++i)
  {
    auto& [destinationIp, msg] = m_receivedMessages.front();

    // TODO (haigh) simplify this handling by adding a map of ip to node
    auto iter = m_nodeIdLookup.find(destinationIp);

    if (iter == m_nodeIdLookup.end()) return;

    auto& [_, destinationNodeId] = *iter;

    for (const auto& node : m_nodes)
    {
      if (node->nodeId() == destinationNodeId)
      {
        // TODO (haigh) remove source node ID
        node->receiveMessage(0, msg.clone());
        break;
      }
    }

    m_receivedMessages.pop_front();
  }
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
                                const EncodedMessage& message)
                        {
                          sendMessage(source,
                                      dest,
                                      message);
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
                                const EncodedMessage& message)
                        {
                          sendMessage(source,
                                      dest,
                                      message);
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
                          const EncodedMessage& message)
{
  // Go through each of the nodes that we have and find the destination Ip Address
  auto iter = m_nodeIdLookup.find(destinationIpAddress);

  if (iter == m_nodeIdLookup.end()) return;

  auto& [_, destinationNodeId] = *iter;

  for (const auto& node : m_nodes)
  {
    if (node->nodeId() == destinationNodeId)
    {
      m_receivedMessages.emplace_back(destinationIpAddress, message.clone());
      break;
    }
  }
}

} // namespace odd::io::simulation

