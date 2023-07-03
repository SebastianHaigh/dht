#include "ChordMessaging.h"

namespace chord {

FindSuccessorMessage::FindSuccessorMessage(CommsVersion version, const NodeId& nodeId)
  : Message(version, MessageType::CHORD_FIND_SUCCESSOR)
{
}

FindSuccessorMessage::FindSuccessorMessage(CommsVersion version)
  : Message(version, MessageType::CHORD_FIND_SUCCESSOR, sizeof(NodeId))
{
}

EncodedMessage FindSuccessorMessage::encode()
{
  EncodedMessage encoded = createEncodedMessage();

  // To encoded the payload I will need a different encoding method in comms coder
  
  return std::move(encoded);
}

void FindSuccessorMessage::decode(const EncodedMessage& message)
{

}

[[nodiscard]] const NodeId& FindSuccessorMessage::nodeId() const
{
  return m_nodeId;
}

}
