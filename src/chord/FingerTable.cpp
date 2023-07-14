#include "FingerTable.h"

namespace chord {

void initialiseFingerTable(FingerTable& fingerTable, const NodeId& nodeId)
{
  int i{0};

  for (auto& entry : fingerTable.m_fingers)
  {
    entry.m_interval.m_start = nodeId + NodeId::powerOfTwo(i);
    entry.m_interval.m_end = nodeId + NodeId::powerOfTwo(i + 1);

    i++;
  }
}

} // namespace chord
