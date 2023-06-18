#include "FingerTable.h"

namespace chord {

FingerTable::FingerTable(const NodeId& localNodeId)
  : m_localNodeId{localNodeId}
{
  // Initialize the finger table entries
  for (int i = 0; i < 160; i++)
  {
    m_entries[i].m_interval = calculateIthInterval(i);
  }
}

const FingerTableEntry &FingerTable::operator[](size_t index) const
{
  return m_entries[index];
}

FingerTableEntry &FingerTable::operator[](size_t index)
{
  return m_entries[index];
}

void FingerTable::addEntry(const NodeId& nodeId,
                           const NodeId& start,
                           const NodeId& end,
                           const std::string& ipAddress,
                           const uint16_t port)
{
}

FingerTableInterval FingerTable::calculateIthInterval(int i) const
{
  FingerTableInterval interval;
  interval.m_start = m_localNodeId + NodeId::powerOfTwo(i);
  interval.m_end = m_localNodeId + NodeId::powerOfTwo(i + 1);

  return interval;
}

} // namespace chord
