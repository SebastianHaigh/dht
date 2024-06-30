#include "Node.h"
#include <boost/asio/ip/address.hpp>
#include <iostream>
#include <utility>
#include "EncodedMessage.h"

namespace odd::io::simulation {

Node::Node(int nodeId,
           boost::asio::ip::address ipAddress,
           OnSendCallback onSendCallback)
  : m_nodeId(nodeId),
    m_ipAddress(std::move(ipAddress)),
    m_onSendCallback(std::move(onSendCallback))
{
}

Node::Node(int nodeId,
           boost::asio::ip::address ipAddress,
           OnSendCallback onSendCallback,
           ReceiveHandler receiveHandler)
  : m_nodeId(nodeId),
    m_ipAddress(std::move(ipAddress)),
    m_onSendCallback(std::move(onSendCallback)),
    m_receiveHandler(std::move(receiveHandler))
{
}

void Node::run()
{
}

void Node::registerReceiveHandler(ReceiveHandler nodeReceiveHandler)
{
  m_receiveHandler = std::move(nodeReceiveHandler);
}

void Node::cancelReceiveHandler()
{
  m_receiveHandler = nullptr;
}

void Node::receiveMessage(const boost::asio::ip::address& sourceIpAddress,
                          const EncodedMessage& message)
{
  if (m_receiveHandler)
  {
    m_receiveHandler(sourceIpAddress, message);
  }
}

void Node::sendMessage(const boost::asio::ip::address& destinationIpAddress,
                       EncodedMessage& message) const
{
  m_onSendCallback(m_ipAddress, destinationIpAddress, message);
}

[[nodiscard]] int Node::nodeId() const
{
  return m_nodeId;
}

[[nodiscard]] const boost::asio::ip::address& Node::ip() const
{
  return m_ipAddress;
}

} // namespace odd::io::simulation
