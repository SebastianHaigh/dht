#include "FingerTable.h"

namespace odd::chord {

void initialiseFingerTable(FingerTable& fingerTable, const NodeId& nodeId)
{
  fingerTable.m_next = 0;

  // The first interval starts at nodeId
  fingerTable.m_fingers[0].m_start = nodeId;
  fingerTable.m_fingers[0].m_end = nodeId + NodeId::powerOfTwo(0);
  fingerTable.m_fingers[0].m_nodeId = nodeId;

  for (int i = 1; i < fingerTable.m_fingers.size(); i++)
  {
    fingerTable.m_fingers[i].m_start = fingerTable.m_fingers[i - 1].m_end + NodeId::powerOfTwo(0);
    fingerTable.m_fingers[i].m_end = nodeId + NodeId::powerOfTwo(i);
    fingerTable.m_fingers[i].m_nodeId = nodeId;
  }
}

} // namespace chord
