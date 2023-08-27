#ifndef IO_TCP_CLIENT_MANAGER_H_
#define IO_TCP_CLIENT_MANAGER_H_

#include <condition_variable>
#include <netinet/in.h>
#include <string>
#include <memory>
#include <sys/socket.h>
#include <unordered_map>
#include <vector>

namespace odd::io::tcp {

class ClientRecord
{
  public:
    ClientRecord(int fd, sockaddr_in socketAddress, socklen_t addressLength);

    const std::string socketName();
    const std::string& socketName() const;

    int m_fd;
    sockaddr_in m_socketAddress;
    socklen_t m_socketAddressLength;
    std::string m_name;
};

class ClientManager
{
  public:
    void processNewClient(std::unique_ptr<ClientRecord> client);

    void processClientDisconnecion(int clientFD);

    std::vector<int> getAllClientFds();

    bool hasClient(int clientFD);

    std::string getClientName(int clientFD);

  private:
    std::mutex m_mutex;
    std::condition_variable m_conditionVariable;
    std::unordered_map<int, std::unique_ptr<ClientRecord>> m_clients;
};

} // namespace odd::io::tcp

#endif // IO_TCP_CLIENT_MANAGER_H_

