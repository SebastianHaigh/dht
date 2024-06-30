#ifndef IO_SIMULATION_NODE_H_
#define IO_SIMULATION_NODE_H_

#include <boost/asio/ip/address.hpp>
#include <cstdint>
#include <functional>
#include <EncodedMessage.h>

namespace odd::io::simulation {

class Node
{
  public:
    using OnSendCallback = std::function<void(const boost::asio::ip::address&,
                                              const boost::asio::ip::address&,
                                              const EncodedMessage&)>;
    using ReceiveHandler = std::function<void(const boost::asio::ip::address&,
                                              const EncodedMessage&)>;
    Node(int nodeId,
         boost::asio::ip::address ipAddress,
         OnSendCallback onSendCallback);

    Node(int nodeId,
         boost::asio::ip::address ipAddress,
         OnSendCallback onSendCallback,
         ReceiveHandler receiveHandler);

    void run();

    void registerReceiveHandler(ReceiveHandler nodeReceiveHandler);

    void cancelReceiveHandler();

    void receiveMessage(const boost::asio::ip::address& sourceIpAddress,
                        const EncodedMessage& message);

    void sendMessage(const boost::asio::ip::address& destinationIpAddress,
                     EncodedMessage& message) const;

    [[nodiscard]] int nodeId() const;
    [[nodiscard]] const boost::asio::ip::address& ip() const;

  private:
    int m_nodeId;
    const boost::asio::ip::address m_ipAddress;
    OnSendCallback m_onSendCallback;
    ReceiveHandler m_receiveHandler;
};

} // namespace odd::io::simulation

#endif // IO_SIMULATION_NODE_H_
