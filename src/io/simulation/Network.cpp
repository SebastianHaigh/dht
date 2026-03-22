#include "Network.h"

#include <arpa/inet.h>
#include <boost/asio/ip/address.hpp>
#include <cstdint>
#include <iostream>
#include <memory>
#include <utility>
#include <EncodedMessage.h>

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
    node->receiveMessage({}, msg.clone());
    m_receivedMessages.pop_front();
  }
}

Node& Network::addNode(const std::string& ipAddress)
{
  return addNode(boost::asio::ip::make_address(ipAddress));
}

Node& Network::addNode(const std::string& ipAddress, Node::ReceiveHandler receiveHandler)
{
  return addNode(boost::asio::ip::make_address(ipAddress), std::move(receiveHandler));
}

Node& Network::addNode(const boost::asio::ip::address& ipAddress)
{
  auto nodeId = m_nextNodeId++;

  auto node_p = std::make_unique<Node>(nodeId,
                                       ipAddress,
                                       createOnSendCallback());

  auto [node, success] = m_nodes.emplace(ipAddress, std::move(node_p));

  return *node->second;
}

Node& Network::addNode(const boost::asio::ip::address& ipAddress, Node::ReceiveHandler receiveHandler)
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
  return [this] (const boost::asio::ip::address& source,
                 const boost::asio::ip::address& dest,
                 const EncodedMessage& message)
  {
    sendMessage(source,
                dest,
                message);
  };
}

void Network::sendMessage(const boost::asio::ip::address& sourceIpAddress,
                          const boost::asio::ip::address& destinationIpAddress,
                          const EncodedMessage& message)
{
  // Go through each of the nodes that we have and find the destination Ip Address
  auto iter = m_nodes.find(destinationIpAddress);

  if (iter == m_nodes.end()) return;

  m_receivedMessages.emplace_back(destinationIpAddress, message.clone());
}

} // namespace odd::io::simulation

