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

  auto* payload_p = &encoded.m_message[2 + 4 + 2];

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

  auto* payload_p = &encoded.m_message[2 + 4 + 2];

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

[[nodiscard]] uint32_t FindSuccessorResponseMessage::requestId() const
{
  return m_requestId;
}

}
