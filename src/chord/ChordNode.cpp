#include "ChordNode.h"

ChordNode::ChordNode(std::string ip, uint16_t port)
{
  // Create a SHA1 hash of the IP and port
  SHA1Hash digest;
  sha1((uint8_t*) ip.c_str(), ip.length(), digest);

  // Create a NodeId from the digest
  m_id = NodeId{ digest };

  // Set the predecessor and successor to be the node itself
  m_predecessor = m_id;
  m_successor = m_id;

  // Set the predecessor and successor IP addresses to be the node itself
  m_predecessorIpAddress = ip;
  m_successorIpAddress = ip;
}

void ChordNode::join(const ChordNode &node)
{

}

const NodeId &ChordNode::getPredecessorId()
{
  return m_predecessor;
}

const NodeId &ChordNode::getSuccessorId()
{
  return m_successor;
}

const IpAddress &ChordNode::getPredecessorIpAddress()
{
  return m_predecessorIpAddress;
}

const IpAddress &ChordNode::getSuccessorIpAddress()
{
  return m_successorIpAddress;
}

const NodeId &ChordNode::findSuccessor(const NodeId &id)
{
  return m_successor;
}

const NodeId &ChordNode::findPredecessor(const NodeId &id)
{
  // Contact nodes (how am I supposed to implement this?) 

  // I think that this request needs to be routed off this node on the nodes returned by closest Preceing finger

  const NodeId &currentNodeId = m_id;
  
  bool check{ true };
  while (check)
  {
    // TODO (haigh) I order to implement this function I am going to need to store a better model of the node that just the node Id
  }
}


const NodeId &ChordNode::closestPrecedingFinger(const NodeId &id)
{
  for (int i = 159; i >= 0; i--)
  {
    if (m_fingerTable[i].m_nodeId > m_id && m_fingerTable[i].m_nodeId < id)
    {
      return m_fingerTable[i].m_nodeId;
    }
  }

  return m_id;
}

void ChordNode::initialiseFingerTable(const ChordNode &node)
{
  
}

void ChordNode::updateOthers()
{

}

void ChordNode::updateFingerTable(const ChordNode &node, uint16_t i)
{

}
