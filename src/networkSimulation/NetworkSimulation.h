#ifndef NETWORKSIMULATOR_H_
#define NETWORKSIMULATOR_H_

#include "../tcp/TcpServer.h"
#include <unordered_map>

// SimulatedNode The purpose of this class is to simulate a single node in a network.
// We can pass it messages and it will send them to other simulated nodes
// It also has a receive function that allows it toreceive messages that have been passed to it 
// by other nodes

using OnSendCallback = std::function<void(uint32_t, uint32_t, int)>;

class SimulatedNode {
  public:
    SimulatedNode(int nodeId, int ipAddress, OnSendCallback onSendCallback);
    virtual ~SimulatedNode() = default;
    void run();
    int nodeId() const;

    void receiveMessage(int source, int destination, int message);

    // The send message function needs to pass the message to the network, which can then find the recipient and pass them the message
    void sendMessage(uint32_t sourceIpAddress, uint32_t destinationIpAddress, int message);

  private:
    int m_nodeId;
    int m_ipAddress;
    OnSendCallback m_onSendCallback;
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
    void run();
    const SimulatedNode& addNode(uint32_t ipAddress);
    void sendMessage(uint32_t sourceIpAddress, uint32_t destinationIpAddress, int message);

  private:
    void removeAllLinksForNode(int nodeId); // I think this could be used as a call back in the node destructor
    std::vector<int> m_nodeIds; // sorted list of nodeIds (which are ints starting from 0)
    std::vector<std::unique_ptr<SimulatedNode>> m_nodes;
    std::vector<SimulatedLink> m_links;
    int m_nextNodeId;

    // TODO (haigh) use this node id lookup to store a mapping between ipAddresses and node ids
    std::unordered_map<uint32_t, int> m_nodeIdLookup;
};

class MockTcpServer : public TcpServer_I
{
  public:
    MockTcpServer(std::string ipAddress);
    ~MockTcpServer();

    void start() override;
    void stop() override;

    void subscribeToAll(std::function<void(int, std::string)> callback) override;
    void broadcast(const std::string& message) override;
    void multicast(const std::string& message, std::vector<int> fds) override;
    void unicast(const std::string& message, int fd) override;
};



#endif // NETWORKSIMULATOR_H_
