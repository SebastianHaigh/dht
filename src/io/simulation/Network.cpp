#include "Network.h"

#include <arpa/inet.h>
#include <cstdint>
#include <memory>
#include <utility>
#include "EncodedMessage.h"

namespace odd::io::simulation {

Network::Network()
  : m_nextNodeId(0)
{
}

boost::asio::io_context& Network::getIoContext()
{
  return m_context;
}

void Network::run()
{
  m_context.poll_one();

  if (m_receivedMessages.empty()) return;

  // Send all of the messages that were in this queue when the method was called
  auto toSend = m_receivedMessages.size();

  for (int i = 0; i < toSend; ++i)
  {
    auto& [destinationIp, msg] = m_receivedMessages.front();
    auto iter = m_nodes.find(destinationIp);

    if (iter == m_nodes.end())
    {
      m_receivedMessages.pop_front();
      continue;
    }

    auto& [_, node] = *iter;

    // TODO (haigh) remove source node ID
    node->receiveMessage(0, msg.clone());
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
  auto nodeId = m_nextNodeId++;

  auto node_p = std::make_unique<Node>(nodeId,
                                       ipAddress,
                                       createOnSendCallback());

  auto [node, success] = m_nodes.emplace(ipAddress, std::move(node_p));

  return *node->second;
}

Node& Network::addNode(uint32_t ipAddress, Node::ReceiveHandler receiveHandler)
{
  auto nodeId = m_nextNodeId++;

  auto node_p = std::make_unique<Node>(nodeId,
                                       ipAddress,
                                       createOnSendCallback(),
                                       std::move(receiveHandler));

  auto [node, success] = m_nodes.emplace(ipAddress, std::move(node_p));

  return *node->second;
}

Node::OnSendCallback Network::createOnSendCallback()
{
  return [this] (uint32_t source,
                 uint32_t dest,
                 const EncodedMessage& message)
  {
    sendMessage(source,
                dest,
                message);
  };
}

void Network::sendMessage(uint32_t sourceIpAddress,
                          uint32_t destinationIpAddress,
                          const EncodedMessage& message)
{
  // Go through each of the nodes that we have and find the destination Ip Address
  auto iter = m_nodes.find(destinationIpAddress);

  if (iter == m_nodes.end()) return;

  m_receivedMessages.emplace_back(destinationIpAddress, message.clone());
}

} // namespace odd::io::simulation

