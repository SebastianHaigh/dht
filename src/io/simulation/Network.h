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
#include <boost/asio/ip/address.hpp>

namespace odd::io::simulation {

class Network {
  public:
    Network();

    void run();

    boost::asio::io_context& getIoContext();

    Node& addNode(const boost::asio::ip::address& ipAddress);
    Node& addNode(const boost::asio::ip::address& ipAddress, Node::ReceiveHandler receiveHandler);
    Node& addNode(const std::string& ipAddress);
    Node& addNode(const std::string& ipAddress, Node::ReceiveHandler receiveHandler);

    void sendMessage(const boost::asio::ip::address& sourceIpAddress,
                     const boost::asio::ip::address& destinationIpAddress,
                     const EncodedMessage& message);

  private:
    Node::OnSendCallback createOnSendCallback();

    boost::asio::io_context m_context;
    int m_nextNodeId;
    std::unordered_map<boost::asio::ip::address, std::unique_ptr<Node>> m_nodes;
    std::deque<std::pair<boost::asio::ip::address, EncodedMessage>> m_receivedMessages;
};

} // namespace odd::io::simulation

#endif // IO_SIMULATION_NETWORK_H_
