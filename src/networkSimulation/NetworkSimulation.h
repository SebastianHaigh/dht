#ifndef NETWORKSIMULATOR_H_
#define NETWORKSIMULATOR_H_

#include "../tcp/TcpServer.h"
#include <cstdint>
#include <unordered_map>

// SimulatedNode The purpose of this class is to simulate a single node in a network.
// We can pass it messages and it will send them to other simulated nodes
// It also has a receive function that allows it toreceive messages that have been passed to it 
// by other nodes

using OnSendCallback = std::function<void(uint32_t, uint32_t, uint8_t*, size_t)>;
using NodeReceiveHandler = std::function<void(uint32_t, uint8_t*, size_t)>;

class SimulatedNode {
  public:
    SimulatedNode(int nodeId, uint32_t ipAddress, OnSendCallback onSendCallback);
    SimulatedNode(int nodeId, uint32_t ipAddress, OnSendCallback onSendCallback, NodeReceiveHandler receiveHandler);
    virtual ~SimulatedNode() = default;
    void run();
    int nodeId() const;

    void registerReceiveHandler(NodeReceiveHandler nodeReceiveHandler);

    void receiveMessage(uint32_t sourceIpAddress, uint8_t* message, size_t messageLength);

    // The send message function needs to pass the message to the network, which can then find the recipient and pass them the message
    void sendMessage(uint32_t destinationIpAddress, uint8_t* message, size_t messageLength) const;

  private:
    int m_nodeId;
    uint32_t m_ipAddress;
    OnSendCallback m_onSendCallback;
    NodeReceiveHandler m_receiveHandler;
};

// SimulatedLink holds references to two nodes and manages the connection between them
// Maybe a good way for this to work is to have some queues inside the link that hold the messages going in each direction
class SimulatedLink {
  public:
    SimulatedLink(const SimulatedNode* node1, const SimulatedNode* node2);

    // The run method will run this link
    void run();

    bool hasNodeId(int nodeId) const;
    // TODO (haigh) latency?

  private:
    const SimulatedNode* m_node1;
    const SimulatedNode* m_node2;
    bool m_up;
};

class NetworkSimulator {
  public:
    NetworkSimulator();
    virtual ~NetworkSimulator() = default;
    void run(); // TODO (haigh) is this method even needed?
    SimulatedNode& addNode(uint32_t ipAddress);
    SimulatedNode& addNode(uint32_t ipAddress, NodeReceiveHandler receiveHandler);
    SimulatedNode& addNode(const std::string& ipAddress);
    SimulatedNode& addNode(const std::string& ipAddress, NodeReceiveHandler receiveHandler);
    void sendMessage(uint32_t sourceIpAddress,
                     uint32_t destinationIpAddress,
                     uint8_t* message,
                     size_t messageLength);

  private:
    void removeAllLinksForNode(int nodeId); // I think this could be used as a call back in the node destructor
    std::vector<int> m_nodeIds; // sorted list of nodeIds (which are ints starting from 0)
    std::vector<std::unique_ptr<SimulatedNode>> m_nodes;
    std::vector<SimulatedLink> m_links;
    int m_nextNodeId;

    // TODO (haigh) use this node id lookup to store a mapping between ipAddresses and node ids
    std::unordered_map<uint32_t, int> m_nodeIdLookup;
};

class MockTcpServer : public tcp::TcpServer_I
{
  public:
    MockTcpServer(std::string ipAddress);
    ~MockTcpServer();

    void start() override;
    void stop() override;

    void subscribeToAll(tcp::OnReceiveCallback callback) override;
    void broadcast(const std::string& message) override;
    void multicast(const std::string& message, std::vector<int> fds) override;
    void unicast(const std::string& message, int fd) override;
};



#endif // NETWORKSIMULATOR_H_
