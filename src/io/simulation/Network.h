#ifndef IO_SIMULATION_NETWORK_H_
#define IO_SIMULATION_NETWORK_H_

#include <EncodedMessage.h>
#include "Node.h"

#include <deque>
#include <functional>
#include <cstdint>
#include <string>
#include <memory>
#include <unordered_map>

namespace odd::io::simulation {

class Network {
  public:
    Network();
    virtual ~Network() = default;
    void run(); // TODO (haigh) is this method even needed?
    Node& addNode(uint32_t ipAddress);
    Node& addNode(uint32_t ipAddress, Node::ReceiveHandler receiveHandler);
    Node& addNode(const std::string& ipAddress);
    Node& addNode(const std::string& ipAddress, Node::ReceiveHandler receiveHandler);
    void sendMessage(uint32_t sourceIpAddress,
                     uint32_t destinationIpAddress,
                     const EncodedMessage& message);

  private:
    std::vector<int> m_nodeIds; // sorted list of nodeIds (which are ints starting from 0)
    std::vector<std::unique_ptr<Node>> m_nodes;
    int m_nextNodeId;

    // TODO (haigh) use this node id lookup to store a mapping between ipAddresses and node ids
    std::unordered_map<uint32_t, int> m_nodeIdLookup;

    std::deque<std::pair<uint32_t, EncodedMessage>> m_receivedMessages;
};

} // namespace odd::io::simulation

#endif // IO_SIMULATION_NETWORK_H_
