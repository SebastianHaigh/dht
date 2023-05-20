#ifndef NETWORKSIMULATOR_H_
#define NETWORKSIMULATOR_H_

#include "../tcp/TcpServer.h"

class SimulatedNode {
  public:
    SimulatedNode(int nodeId, int ipAddress);
    virtual ~SimulatedNode();
    void run();

    void receiveMessage(int source, int destination, int message);
    void sendMessage(int source, int destination, int message);

  private:
    int m_nodeId;
    int m_ipAddress;
    MockTcpServer m_tcpServer;
};

class SimulatedLink {
  public:
    SimulatedLink(const SimulatedNode& node1, const SimulatedNode& node2);
    virtual ~SimulatedLink();
    void run();

  private:
    const SimulatedNode& node1;
    const SimulatedNode& node2;
};

class NetworkSimulator {
  public:
    NetworkSimulator();
    virtual ~NetworkSimulator();
    void run();
    void addNode(SimulatedNode node);
    void addLink(SimulatedLink link);

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
