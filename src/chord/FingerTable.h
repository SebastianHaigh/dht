#ifndef FINGER_TABLE_H_
#define FINGER_TABLE_H_

#include <array>

#include "NodeId.h"

namespace odd::chord {

struct FingerTableEntry
{
  NodeId m_start;
  NodeId m_end;
  NodeId m_nodeId;
};

struct FingerTable
{
  std::array<FingerTableEntry, 160> m_fingers;
  NodeId m_localNodeId;
  std::size_t m_next;
};

void initialiseFingerTable(FingerTable& fingerTable, const NodeId& nodeId);

} // namespace chord

#endif // FINGER_TABLE_H_

