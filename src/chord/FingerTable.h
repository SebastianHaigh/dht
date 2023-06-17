#ifndef FINGER_TABLE_H_
#define FINGER_TABLE_H_

#include <array>

#include "NodeId.h"

namespace chord {

struct FingerTableInterval
{
  NodeId m_start;
  NodeId m_end;
};

struct FingerTableEntry
{
  FingerTableInterval m_interval;
  NodeId m_nodeId;
  uint32_t m_ipAddress;
  uint16_t m_port;
};

class FingerTable
{
  public:
    explicit FingerTable(const NodeId& localNodeId);

    void addEntry(const NodeId& nodeId,
                  const NodeId& start,
                  const NodeId& end,
                  const std::string& ipAddress,
                  uint16_t port);

    const FingerTableEntry &operator[](size_t index) const;
    FingerTableEntry &operator[](size_t index);

  private:
    FingerTableInterval calculateIthInterval(int i) const;

    std::array<FingerTableEntry, 160> m_entries;
    NodeId m_localNodeId;
    
};

} // namespace chord

#endif // FINGER_TABLE_H_

