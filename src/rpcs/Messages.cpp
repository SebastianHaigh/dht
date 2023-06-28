#include "Messages.h"
#include <cstdint>
#include <cstring>

PositionMessage::PositionMessage() : m_id(0), m_neighbours{0} {};

PositionMessage::PositionMessage(NodeId id, const NodeId* neighbours, size_t numNeighbours)
  : m_id(id),
    m_neighbours{0}
{
  numNeighbours = (numNeighbours > 5) ? 5 : numNeighbours;

  for (int i = 0; i < numNeighbours; i++)
  {
    m_neighbours[i] = neighbours[i];
  }
}

PositionMessage::PositionMessage(NodeId id, std::vector<NodeId> neighbours)
  : m_id(id),
    m_neighbours{0}
{
  for (int i = 0; i < 5; i++)
  {
    m_neighbours[i] = neighbours[i];
  }
}

size_t PositionMessage::encode(uint8_t* buffer, size_t bufferSize) const
{
  if (bufferSize < messageSize()) return 0;

  memcpy(buffer, &m_id, sizeof(NodeId));
  buffer += sizeof(NodeId);
  
  memcpy(buffer, &m_neighbours, sizeof(NodeId) * 5);

  return sizeof(NodeId) * 6;
}

void PositionMessage::decode(uint8_t* buffer, size_t bufferSize)
{
  if (bufferSize < messageSize()) return; // TODO (haigh) does this need to return some sort of error?

  m_id = *reinterpret_cast<NodeId*>(buffer);
  buffer += sizeof(NodeId); // Move the pointer onto the first neighbour

  memcpy(&m_neighbours, buffer, sizeof(NodeId) * 5);
}


JoinMessage::JoinMessage() : m_id(0), m_ip(0) {};

JoinMessage::JoinMessage(NodeId id, uint32_t ip)
  : m_id(id),
    m_ip(ip)
{
}

size_t JoinMessage::encode(uint8_t* buffer, size_t bufferSize) const
{
  if (bufferSize < messageSize()) return 0;

  memcpy(buffer, &m_type, sizeof(MessageType));
  buffer += sizeof(MessageType);

  memcpy(buffer, &m_id, sizeof(NodeId));
  buffer += sizeof(NodeId);

  memcpy(buffer, &m_ip, sizeof(uint32_t));

  return messageSize();
}

void JoinMessage::decode(uint8_t* buffer, size_t bufferSize)
{
  if (bufferSize < messageSize()) return; // TODO (haigh) does this need to return some sort of error?
  m_type = *reinterpret_cast<MessageType*>(buffer);

  if (m_type != MessageType::JOIN) return; // TODO (haigh) this is also an error
  buffer += sizeof(MessageType);

  m_id = *reinterpret_cast<NodeId*>(buffer);
  buffer += sizeof(NodeId);

  m_ip = *reinterpret_cast<uint32_t*>(buffer);
}
