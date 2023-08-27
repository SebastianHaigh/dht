#include "Node.h"

namespace odd::io::simulation {

Node::Node(int nodeId, uint32_t ipAddress, OnSendCallback onSendCallback)
  : m_nodeId(nodeId),
    m_ipAddress(ipAddress),
    m_onSendCallback(std::move(onSendCallback))
{
}

Node::Node(int nodeId,
           uint32_t ipAddress,
           OnSendCallback onSendCallback,
           ReceiveHandler receiveHandler)
  : m_nodeId(nodeId),
    m_ipAddress(ipAddress),
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

void Node::receiveMessage(uint32_t sourceIpAddress, uint8_t* message, size_t messageLength)
{
  if (m_receiveHandler)
    m_receiveHandler(sourceIpAddress, message, messageLength);
}

void Node::sendMessage(uint32_t destinationIpAddress,
                                uint8_t* message,
                                size_t messageLength) const
{
  m_onSendCallback(m_ipAddress, destinationIpAddress, message, messageLength);
}

[[nodiscard]] int Node::nodeId() const
{
  return m_nodeId;
}

[[nodiscard]] uint32_t Node::ip() const
{
  return m_ipAddress;
}

} // namespace odd::io::simulation
