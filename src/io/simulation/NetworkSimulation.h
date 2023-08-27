#ifndef NETWORKSIMULATOR_H_
#define NETWORKSIMULATOR_H_

#include "simulation/Node.h"
#include <functional>
#include <cstdint>
#include <unordered_map>

#include <tcp/Server.h>

// Node The purpose of this class is to simulate a single node in a network.
// We can pass it messages and it will send them to other simulated nodes
// It also has a receive function that allows it toreceive messages that have been passed to it 
// by other nodes

namespace odd::io::simulation {


// SimulatedLink holds references to two nodes and manages the connection between them
// Maybe a good way for this to work is to have some queues inside the link that hold the messages going in each direction
class SimulatedLink {
  public:
    SimulatedLink(const Node* node1, const Node* node2);

    // The run method will run this link
    void run();

    bool hasNodeId(int nodeId) const;
    // TODO (haigh) latency?

  private:
    const Node* m_node1;
    const Node* m_node2;
    bool m_up;
};

class NetworkSimulator {
  public:
    NetworkSimulator();
    virtual ~NetworkSimulator() = default;
    void run(); // TODO (haigh) is this method even needed?
    Node& addNode(uint32_t ipAddress);
    Node& addNode(uint32_t ipAddress, Node::ReceiveHandler receiveHandler);
    Node& addNode(const std::string& ipAddress);
    Node& addNode(const std::string& ipAddress, Node::ReceiveHandler receiveHandler);
    void sendMessage(uint32_t sourceIpAddress,
                     uint32_t destinationIpAddress,
                     uint8_t* message,
                     size_t messageLength);

  private:
    void removeAllLinksForNode(int nodeId); // I think this could be used as a call back in the node destructor
    std::vector<int> m_nodeIds; // sorted list of nodeIds (which are ints starting from 0)
    std::vector<std::unique_ptr<Node>> m_nodes;
    std::vector<SimulatedLink> m_links;
    int m_nextNodeId;

    // TODO (haigh) use this node id lookup to store a mapping between ipAddresses and node ids
    std::unordered_map<uint32_t, int> m_nodeIdLookup;
};

class MockTcpServer : public io::tcp::Server_I
{
  public:
    MockTcpServer(std::string ipAddress);
    ~MockTcpServer();

    void start() override;
    void stop() override;

    void subscribeToAll(io::tcp::OnReceiveCallback callback) override;
    void broadcast(const std::string& message) override;
    void multicast(const std::string& message, std::vector<int> fds) override;
    void unicast(const std::string& message, int fd) override;
};

} // namespace odd::io::simulation

#endif // NETWORKSIMULATOR_H_
