#ifndef CHORD_NODE_H_
#define CHORD_NODE_H_

#include "../hashing/sha1.h"

struct NodeId
{
  NodeId() = default;
  NodeId(const uint8_t id[20])
  {
    memcpy(m_id, id, 20);
  }
  SHA1Hash m_id;

  bool operator==(const NodeId& other) const
  {
    return memcmp(m_id, other.m_id, 20) == 0;
  }

  bool operator<(const NodeId& other) const
  {
    return memcmp(m_id, other.m_id, 20) < 0;
  }

  bool operator>(const NodeId& other) const
  {
    return memcmp(m_id, other.m_id, 20) > 0;
  }
};

struct FingerTableEntry
{
  NodeId m_nodeId;
  std::string m_ipAddress;
  uint16_t m_port;
};

class FingerTable
{
  public:
    void addEntry(const NodeId& nodeId,
                  const std::string& ipAddress,
                  const uint16_t port);

    void removeEntry(const NodeId& nodeId);

    const FingerTableEntry &operator[](uint16_t index) const { return m_entries[index]; }

  private:
    FingerTableEntry m_entries[160];
};

using IpAddress = std::string;

class ChordNode
{
  public:
    // TODO (haigh) The ChordNode is going to need some sort of tcp endpoint so that it can communicate with the other nodes
    ChordNode(std::string ip, uint16_t port);

    void join(const ChordNode& node);
    NodeId& getId();

    const NodeId& getPredecessorId();
    const NodeId& getSuccessorId();

    const IpAddress& getPredecessorIpAddress();
    const IpAddress& getSuccessorIpAddress();

    const NodeId& findSuccessor(const NodeId& id);
    const NodeId& findPredecessor(const NodeId& id);
    const NodeId& closestPrecedingFinger(const NodeId& id);

  private:
    void initialiseFingerTable(const ChordNode& node);
    void updateOthers();
    void updateFingerTable(const ChordNode& node, uint16_t i);

    NodeId m_id;
    NodeId m_predecessor;
    NodeId m_successor;
    IpAddress m_predecessorIpAddress;
    IpAddress m_successorIpAddress;
    FingerTable m_fingerTable;
};

#endif // CHORD_NODE_H_

