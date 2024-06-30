#ifndef IO_SIMULATION_NETWORK_H_
#define IO_SIMULATION_NETWORK_H_

#include <EncodedMessage.h>
#include "Node.h"

#include <deque>
#include <cstdint>
#include <string>
#include <memory>
#include <unordered_map>
#include <boost/asio/io_context.hpp>

namespace odd::io::simulation {

class Network {
  public:
    Network();

    void run();

    boost::asio::io_context& getIoContext();

    Node& addNode(uint32_t ipAddress);
    Node& addNode(uint32_t ipAddress, Node::ReceiveHandler receiveHandler);
    Node& addNode(const std::string& ipAddress);
    Node& addNode(const std::string& ipAddress, Node::ReceiveHandler receiveHandler);

    void sendMessage(uint32_t sourceIpAddress,
                     uint32_t destinationIpAddress,
                     const EncodedMessage& message);

  private:
    Node::OnSendCallback createOnSendCallback();

    boost::asio::io_context m_context;
    int m_nextNodeId;
    std::unordered_map<uint32_t, std::unique_ptr<Node>> m_nodes;
    std::deque<std::pair<uint32_t, EncodedMessage>> m_receivedMessages;
};

} // namespace odd::io::simulation

#endif // IO_SIMULATION_NETWORK_H_
