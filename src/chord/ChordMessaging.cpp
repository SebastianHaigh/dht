#include "ChordMessaging.h"
#include "../comms/CommsCoder.h"

namespace chord {

FindSuccessorMessage::FindSuccessorMessage(CommsVersion version, const NodeId& nodeId)
  : Message(version, MessageType::CHORD_FIND_SUCCESSOR),
    m_nodeId(nodeId)
{
}

FindSuccessorMessage::FindSuccessorMessage(CommsVersion version)
  : Message(version, MessageType::CHORD_FIND_SUCCESSOR, sizeof(NodeId))
{
}

[[nodiscard]] EncodedMessage FindSuccessorMessage::encode() const
{
  EncodedMessage encoded = createEncodedMessage();

  auto* payload_p = &encoded.m_message[2 + 4 + 2];

  encodeSingleValue(&m_nodeId, payload_p);
  
  return std::move(encoded);
}

void FindSuccessorMessage::decode(const EncodedMessage& message)
{
  decodeHeaders(message);

  auto* payload_p = &message.m_message[8];

  decodeSingleValue(payload_p, &m_nodeId);
}

[[nodiscard]] const NodeId& FindSuccessorMessage::nodeId() const
{
  return m_nodeId;
}

}
