#ifndef IO_SIMULATION_NODE_H_
#define IO_SIMULATION_NODE_H_

#include <cstdint>
#include <functional>

namespace odd::io::simulation {

class Node
{
  public:
    using OnSendCallback = std::function<void(uint32_t, uint32_t, uint8_t*, size_t)>;
    using ReceiveHandler = std::function<void(uint32_t, uint8_t*, size_t)>;
    Node(int nodeId, uint32_t ipAddress, OnSendCallback onSendCallback);

    Node(int nodeId,
         uint32_t ipAddress,
         OnSendCallback onSendCallback,
         ReceiveHandler receiveHandler);

    void run();

    void registerReceiveHandler(ReceiveHandler nodeReceiveHandler);


    void cancelReceiveHandler();


    void receiveMessage(uint32_t sourceIpAddress, uint8_t* message, size_t messageLength);

    void sendMessage(uint32_t destinationIpAddress,
                     uint8_t* message,
                     size_t messageLength) const;

    [[nodiscard]] int nodeId() const;
    [[nodiscard]] uint32_t ip() const;

  private:
    int m_nodeId;
    uint32_t m_ipAddress;
    OnSendCallback m_onSendCallback;
    ReceiveHandler m_receiveHandler;
};

} // namespace odd::io::simulation

#endif // IO_SIMULATION_NODE_H_
