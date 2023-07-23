#include "ChordMessaging.h"
#include "../comms/CommsCoder.h"

namespace chord {

FindSuccessorMessage::FindSuccessorMessage(CommsVersion version,
                                           const NodeId& nodeId,
                                           const NodeId& sourceNodeId,
                                           uint32_t requestId)
  : Message(version, MessageType::CHORD_FIND_SUCCESSOR, 2 * sizeof(NodeId) + 4),
    m_nodeIdForQuery(nodeId),
    m_sourceNodeId(sourceNodeId),
    m_requestId(requestId)
{
}

FindSuccessorMessage::FindSuccessorMessage(CommsVersion version)
  : Message(version, MessageType::CHORD_FIND_SUCCESSOR, 2 * sizeof(NodeId) + 4),
    m_requestId(0)
{
}

[[nodiscard]] EncodedMessage FindSuccessorMessage::encode() const
{
  EncodedMessage encoded = createEncodedMessage();

  auto* payload_p = &encoded.m_message[8];

  encodeSingleValue(&m_nodeIdForQuery, payload_p);
  payload_p += sizeof(NodeId);

  encodeSingleValue(&m_sourceNodeId, payload_p);
  payload_p += sizeof(NodeId);

  encodeSingleValue(&m_requestId, payload_p);

  return std::move(encoded);
}

void FindSuccessorMessage::decode(const EncodedMessage& message)
{
  decodeHeaders(message);

  auto* payload_p = &message.m_message[8];

  decodeSingleValue(payload_p, &m_nodeIdForQuery);
  payload_p += sizeof(NodeId);

  decodeSingleValue(payload_p, &m_sourceNodeId);
  payload_p += sizeof(NodeId);

  decodeSingleValue(payload_p, &m_requestId);
}

[[nodiscard]] const NodeId& FindSuccessorMessage::queryNodeId() const
{
  return m_nodeIdForQuery;
}

[[nodiscard]] const NodeId& FindSuccessorMessage::sourceNodeId() const
{
  return m_sourceNodeId;
}

[[nodiscard]] uint32_t FindSuccessorMessage::requestId() const
{
  return m_requestId;
}

FindSuccessorResponseMessage::FindSuccessorResponseMessage(CommsVersion version,
                                                           const NodeId& nodeId,
                                                           const NodeId& sourceNodeId,
                                                           uint32_t requestId)
  : Message(version, MessageType::CHORD_FIND_SUCCESSOR_RESPONSE, 2 * sizeof(NodeId) + 4),
    m_nodeId(nodeId),
    m_sourceNodeId(sourceNodeId),
    m_requestId(requestId)
{
}

FindSuccessorResponseMessage::FindSuccessorResponseMessage(CommsVersion version)
  : Message(version, MessageType::CHORD_FIND_SUCCESSOR_RESPONSE, 2 * sizeof(NodeId) + 4),
    m_requestId(0)
{
}

[[nodiscard]] EncodedMessage FindSuccessorResponseMessage::encode() const
{
  EncodedMessage encoded = createEncodedMessage();

  auto* payload_p = &encoded.m_message[8];

  encodeSingleValue(&m_nodeId, payload_p);
  payload_p += sizeof(NodeId);

  encodeSingleValue(&m_sourceNodeId, payload_p);
  payload_p += sizeof(NodeId);

  encodeSingleValue(&m_requestId, payload_p);

  return std::move(encoded);
}

void FindSuccessorResponseMessage::decode(const EncodedMessage& message)
{
  decodeHeaders(message);

  auto* payload_p = &message.m_message[8];

  decodeSingleValue(payload_p, &m_nodeId);
  payload_p += sizeof(NodeId);

  decodeSingleValue(payload_p, &m_sourceNodeId);
  payload_p += sizeof(NodeId);

  decodeSingleValue(payload_p, &m_requestId);
}

[[nodiscard]] const NodeId& FindSuccessorResponseMessage::nodeId() const
{
  return m_nodeId;
}

[[nodiscard]] const NodeId& FindSuccessorResponseMessage::sourceNodeId() const
{
  return m_sourceNodeId;
}

[[nodiscard]] uint32_t FindSuccessorResponseMessage::requestId() const
{
  return m_requestId;
}

NotifyMessage::NotifyMessage(CommsVersion version,
                             const NodeId& nodeId)
  : Message(version, MessageType::CHORD_NOTIFY, sizeof(NodeId)),
    m_nodeId(nodeId)
{
}

NotifyMessage::NotifyMessage(CommsVersion version)
  : Message(version, MessageType::CHORD_NOTIFY, sizeof(NodeId))
{
}

[[nodiscard]] EncodedMessage NotifyMessage::encode() const
{
  EncodedMessage encoded = createEncodedMessage();

  auto* payload_p = &encoded.m_message[8];

  encodeSingleValue(&m_nodeId, payload_p);
  payload_p += sizeof(NodeId);

  return std::move(encoded);
}

void NotifyMessage::decode(const EncodedMessage& message)
{
  decodeHeaders(message);

  auto* payload_p = &message.m_message[8];

  decodeSingleValue(payload_p, &m_nodeId);
  payload_p += sizeof(NodeId);
}

[[nodiscard]] const NodeId& NotifyMessage::nodeId() const
{
  return m_nodeId;
}

GetNeighboursMessage::GetNeighboursMessage(CommsVersion version,
                                           const NodeId& sourceNodeId,
                                           uint32_t requestId)
  : Message(version, MessageType::CHORD_GET_NEIGHBOURS, sizeof(uint32_t) + sizeof(NodeId)),
    m_sourceNodeId(sourceNodeId),
    m_requestId(requestId)
{
}

GetNeighboursMessage::GetNeighboursMessage(CommsVersion version)
  : Message(version, MessageType::CHORD_GET_NEIGHBOURS, sizeof(uint32_t) + sizeof(NodeId)),
    m_requestId(0)
{
}

[[nodiscard]] EncodedMessage GetNeighboursMessage::encode() const
{
  EncodedMessage encoded = createEncodedMessage();

  auto* payload_p = &encoded.m_message[8];

  encodeSingleValue(&m_sourceNodeId, payload_p);
  payload_p += sizeof(NodeId);

  encodeSingleValue(&m_requestId, payload_p);

  return std::move(encoded);
}

void GetNeighboursMessage::decode(const EncodedMessage& message)
{
  decodeHeaders(message);

  auto* payload_p = &message.m_message[8];

  decodeSingleValue(payload_p, &m_sourceNodeId);
  payload_p += sizeof(NodeId);

  decodeSingleValue(payload_p, &m_requestId);
}

[[nodiscard]] const NodeId& GetNeighboursMessage::sourceNodeId() const
{
  return m_sourceNodeId;
}

[[nodiscard]] uint32_t GetNeighboursMessage::requestId() const
{
  return m_requestId;
}

GetNeighboursResponseMessage::GetNeighboursResponseMessage(CommsVersion version,
                                                           const NodeId& successor,
                                                           const NodeId& predecessor,
                                                           const NodeId& sourceNodeId,
                                                           uint32_t requestId)
  : Message(version, MessageType::CHORD_GET_NEIGHBOURS_RESPONSE, sizeof(uint32_t) + 3 * sizeof(NodeId)),
    m_successor(successor),
    m_predecessor(predecessor),
    m_sourceNodeId(sourceNodeId),
    m_requestId(requestId)
{
}

GetNeighboursResponseMessage::GetNeighboursResponseMessage(CommsVersion version)
  : Message(version, MessageType::CHORD_GET_NEIGHBOURS_RESPONSE, sizeof(uint32_t) + 3 * sizeof(NodeId)),
    m_requestId(0)
{
}

[[nodiscard]] EncodedMessage GetNeighboursResponseMessage::encode() const
{
  EncodedMessage encoded = createEncodedMessage();

  auto* payload_p = &encoded.m_message[8];

  encodeSingleValue(&m_successor, payload_p);
  payload_p += sizeof(NodeId);

  encodeSingleValue(&m_predecessor, payload_p);
  payload_p += sizeof(NodeId);

  encodeSingleValue(&m_sourceNodeId, payload_p);
  payload_p += sizeof(NodeId);

  encodeSingleValue(&m_requestId, payload_p);

  return std::move(encoded);
}

void GetNeighboursResponseMessage::decode(const EncodedMessage& message)
{
  decodeHeaders(message);

  auto* payload_p = &message.m_message[8];

  decodeSingleValue(payload_p, &m_successor);
  payload_p += sizeof(NodeId);

  decodeSingleValue(payload_p, &m_predecessor);
  payload_p += sizeof(NodeId);

  decodeSingleValue(payload_p, &m_sourceNodeId);
  payload_p += sizeof(NodeId);

  decodeSingleValue(payload_p, &m_requestId);
}

[[nodiscard]] const NodeId& GetNeighboursResponseMessage::successor() const
{
  return m_sourceNodeId;
}

[[nodiscard]] const NodeId& GetNeighboursResponseMessage::predecessor() const
{
  return m_sourceNodeId;
}

[[nodiscard]] const NodeId& GetNeighboursResponseMessage::sourceNodeId() const
{
  return m_sourceNodeId;
}

[[nodiscard]] uint32_t GetNeighboursResponseMessage::requestId() const
{
  return m_requestId;
}
}
