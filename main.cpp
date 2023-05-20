#include <cstdint>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <thread>
#include <functional>
#include <unordered_map>

namespace dht {

using IpAddress = uint32_t;

class NetworkEndpoint
{
  public:
    NetworkEndpoint(IpAddress ipAddress)
      : m_ipAddress(ipAddress)
    {
    }


  private:
    IpAddress m_ipAddress;
};

class NetworkConnection
{
  public:
    NetworkConnection(IpAddress ipAddressA, IpAddress ipAddressB)
      : m_ipAddressA(ipAddressA),
        m_ipAddressB(ipAddressB)
    {
    }

  private:
    IpAddress m_ipAddressA;
    IpAddress m_ipAddressB;
};

class SimulatedNetwork
{
  public:
    void addHost(IpAddress ipAddress)
    {

    }

    void send(const IpAddress& sourceIpAddress, const IpAddress& destinationIpAddress, char* data, size_t size)
    {
      
    }
  
  private:
    std::vector<IpAddress> m_knownHosts;
    std::vector<NetworkConnection> m_connections;
};


class JoinRequest {
  public:
    JoinRequest(const IpAddress& sourceIpAddress, const IpAddress& destinationIpAddress)
      : m_sourceIpAddress(sourceIpAddress),
      m_destinationIpAddress(destinationIpAddress)
    {
    }

    IpAddress sourceIpAddress() const { return m_sourceIpAddress; }
    IpAddress destinationIpAddress() const { return m_destinationIpAddress; }
  private:
    IpAddress m_sourceIpAddress;
    IpAddress m_destinationIpAddress;
};

class JoinResponse {
  public:
    JoinResponse(const IpAddress& sourceIpAddress, const IpAddress& destinationIpAddress)
      : m_sourceIpAddress(sourceIpAddress),
        m_destinationIpAddress(destinationIpAddress)
    {
    }

    IpAddress sourceIpAddress() const { return m_sourceIpAddress; }
    IpAddress destinationIpAddress() const { return m_destinationIpAddress; }

  private:
    IpAddress m_sourceIpAddress;
    IpAddress m_destinationIpAddress;
    std::vector<IpAddress> m_ipAddresses;
};

class RoutingTable {

  private:
    std::vector<IpAddress> m_ipAddresses;

};

class Transport {
  public:
    void send(const IpAddress& ipAddress, const JoinRequest& joinRequest);
    void send(const IpAddress& ipAddress, const JoinResponse& joinResponse);
    void receive(const IpAddress& ipAddress, const JoinRequest& joinRequest);
    void receive(const IpAddress& ipAddress, const JoinResponse& joinResponse);

  private:
    IpAddress m_localIpAddress;

};

class AsyncWorkQueue
{
  public:
    AsyncWorkQueue()
      : m_isRunning(true)
    {
      m_thread.emplace_back(&AsyncWorkQueue::workerThreadFunction, this);
    }

    ~AsyncWorkQueue()
    {
      m_isRunning = false;
      m_conditionVariable.notify_all();
      for (auto& thread : m_thread)
      {
        thread.join();
      }
    }

    void addWork(std::function<void()> work)
    {
      std::lock_guard<std::mutex> lock(m_mutex);
      m_work.push(work);
      m_conditionVariable.notify_one();
    }

  private:
    void workerThreadFunction()
    {
      while (m_isRunning)
      {
        waitForWork();
        while (!m_work.empty())
        {
          std::function<void()> work;
          {
            std::lock_guard<std::mutex> lock(m_mutex);
            work = m_work.front();
            m_work.pop();
          }
          work();
        }
      }
    }
    
    void waitForWork()
    {
      std::unique_lock<std::mutex> lock(m_mutex);
      m_conditionVariable.wait(lock, [this] { return !m_work.empty(); });
    }

    std::queue<std::function<void()>> m_work;
    std::mutex m_mutex;
    std::condition_variable m_conditionVariable;
    std::atomic<bool> m_isRunning;
    std::vector<std::thread> m_thread;
};

class DhtNode {
  public:
    DhtNode();
    ~DhtNode();

    /*
     * Join the DHT network.
     * @param ip_address The IP address of a node in the DHT network.
     */
    void join(IpAddress ipAddress)
    {
      // 1. Send a join request to the node.
      m_asyncWorkQueue.addWork([this, ipAddress] { sendJoinRequest(ipAddress); });
    }

    /*
     * Leave the DHT network.
     */
    void leave();

    bool isJoining() const { return m_state == State::JOINING; } 
    bool isJoined() const { return m_state == State::JOINED; }

  private:
    void doJoin(IpAddress ipAddress)
    {
    }

    void sendJoinRequest(const IpAddress& ipAddress)
    {
      int joinAttempts = 0;

      while (m_state == State::JOINING && joinAttempts < 3)
      {
        joinAttempts++;

        JoinRequest joinRequest{ m_localIpAddress, ipAddress };
        m_transport.send(ipAddress, joinRequest);

        std::this_thread::sleep_for(std::chrono::seconds(1));

      }

      if (m_state != State::JOINED)
      {
        m_state = State::JOIN_FAILED;
      }
    }

    void handleJoinResponse(const IpAddress& ipAddress, const JoinResponse& joinResponse)
    {
      if (m_state == State::JOINING)
      {
        m_state = State::JOINED;
        // TODO (haigh): Add the IP addresses to the routing table.
      }



    }
    void handleJoinRequest(const JoinRequest& joinRequest)
    {
      // TODO (haigh): Check if the node is already in the routing table.
      
      // TODO (haigh): Add the node to the routing table.
      
      // TODO (haigh): what should we do if the node receiving the request is currently isolated?

      if (m_state != State::JOINED && m_state != State::JOINING) return;

      JoinResponse joinResponse{ m_localIpAddress, joinRequest.sourceIpAddress() };

      // How to determine which ip addresses to send back?

      m_transport.send(joinRequest.sourceIpAddress(), joinResponse);
    }

    enum class State {
      JOINING,
      JOINED,
      JOIN_FAILED,
      LEAVING,
      LEFT
    };

    State m_state;

    IpAddress m_localIpAddress;
    RoutingTable m_routingTable;
    Transport m_transport;
    AsyncWorkQueue m_asyncWorkQueue;
};

}

int main()
{

}
