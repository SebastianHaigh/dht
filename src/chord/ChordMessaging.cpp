#include "ChordMessaging.h"
#include "../comms/CommsCoder.h"

namespace chord {

FindSuccessorMessage::FindSuccessorMessage(CommsVersion version, 
                                           const NodeId& nodeId,
                                           const NodeId& sourceNodeId)
  : Message(version, MessageType::CHORD_FIND_SUCCESSOR),
    m_nodeIdForQuery(nodeId),
    m_sourceNodeId(sourceNodeId)
{
}

FindSuccessorMessage::FindSuccessorMessage(CommsVersion version)
  : Message(version, MessageType::CHORD_FIND_SUCCESSOR, 2 * sizeof(NodeId))
{
}

[[nodiscard]] EncodedMessage FindSuccessorMessage::encode() const
{
  EncodedMessage encoded = createEncodedMessage();

  auto* payload_p = &encoded.m_message[2 + 4 + 2];

  encodeSingleValue(&m_nodeIdForQuery, payload_p);
  payload_p += sizeof(NodeId);

  encodeSingleValue(&m_sourceNodeId, payload_p);

  return std::move(encoded);
}

void FindSuccessorMessage::decode(const EncodedMessage& message)
{
  decodeHeaders(message);

  auto* payload_p = &message.m_message[8];

  decodeSingleValue(payload_p, &m_nodeIdForQuery);
  payload_p += sizeof(NodeId);

  decodeSingleValue(payload_p, &m_sourceNodeId);
}

[[nodiscard]] const NodeId& FindSuccessorMessage::queryNodeId() const
{
  return m_nodeIdForQuery;
}

[[nodiscard]] const NodeId& FindSuccessorMessage::sourceNodeId() const
{
  return m_sourceNodeId;
}

}