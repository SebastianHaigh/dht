#include "RPCs.h"
#include <cstdint>
#include <cstring>
#include <iostream>

Node::Node(SimulatedNode& node, NodeConfig config)
  : m_node(node),
    m_id(config.m_id),
    m_ip(config.m_ip)
{
  auto onReceiveCallback = [this] (uint32_t sourceIpAddress, EncodedMessage message)
                            {
                               receiveMessage(sourceIpAddress, std::move(message));
                            };


  for (unsigned int neighbourIpAddress : config.m_neighbourIpAddresses)
  {
    if (neighbourIpAddress == 0) return;

    m_neighbourIps.push_back(neighbourIpAddress);
  }
};

// When the node is started it needs to reach out to its neighbours. Since this is intended to be P2P
// I don't want to create this in such a way that the nodes know all of their neighbours at start up.
// You are given some nodes to connect to, but those nodes don't know who you are until you connect to them.
// When you connect they will add you to their list of known nodes.
// However, a node could end up with more than five neighbours. So the position message will have to change.
void Node::run()
{
  // Generate join requests

  // Wait for join response
  // Do I need to utilise the threadpool here?

}

void Node::start(const OpType& opType)
{
  switch(opType)
  {
    case OpType::LEARN_COMM_GRAPH:
    {
      break;
    }
    case OpType::BUILD_SPANNING_TREE:
    {
      break;
    }
    default:
    {
      return;
    }
  }
}

void Node::receiveMessage(uint32_t sourceIpAddress, EncodedMessage encodedMessage)
{
  auto messageType_p = reinterpret_cast<MessageType*>(encodedMessage.m_message + 2);

  switch (*messageType_p)
  {
    case MessageType::JOIN:
    {
      JoinMessage message{ CommsVersion::V1 };

      message.decode(encodedMessage);

      handleJoinRequest(message);
      break;
    }
    case MessageType::JOIN_RESPONSE:
    {
      JoinResponseMessage message{ CommsVersion::V1 };

      message.decode(encodedMessage);

      handleJoinResponse(message);
      break;
    }
    case MessageType::POSITION:
    {
      PositionMessage message{ CommsVersion::V1 };

      message.decode(encodedMessage);

      break;
    }
    default:
    {
      return;
    }
  }
}

void Node::handleJoinRequest(const JoinMessage& message)
{
}

void Node::handleJoinResponse(const JoinResponseMessage& message)
{
}

void Node::position(const PositionMessage& positionMessage)
{
}

