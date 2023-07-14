#include <catch2/catch_test_macros.hpp>

#include "../NodeId.h"
#include "../FingerTable.h"

namespace chord { namespace test {

TEST_CASE("Create a finger table")
{
  // All of the required intervals for the finger table of a node at zero
  // are the powers of two from 1 to 2^160, these are defined below as strings:

  std::vector<std::string> intervalStarts = {
    { "0000000000000000000000000000000000000001" },
    { "0000000000000000000000000000000000000002" },
    { "0000000000000000000000000000000000000004" },
    { "0000000000000000000000000000000000000008" },
    { "0000000000000000000000000000000000000010" },
    { "0000000000000000000000000000000000000020" },
    { "0000000000000000000000000000000000000040" },
    { "0000000000000000000000000000000000000080" },
    { "0000000000000000000000000000000000000100" },
    { "0000000000000000000000000000000000000200" },
    { "0000000000000000000000000000000000000400" },
    { "0000000000000000000000000000000000000800" },
    { "0000000000000000000000000000000000001000" },
    { "0000000000000000000000000000000000002000" },
    { "0000000000000000000000000000000000004000" },
    { "0000000000000000000000000000000000008000" },
    { "0000000000000000000000000000000000010000" },
    { "0000000000000000000000000000000000020000" },
    { "0000000000000000000000000000000000040000" },
    { "0000000000000000000000000000000000080000" },
    { "0000000000000000000000000000000000100000" },
    { "0000000000000000000000000000000000200000" },
    { "0000000000000000000000000000000000400000" },
    { "0000000000000000000000000000000000800000" },
    { "0000000000000000000000000000000001000000" },
  };

  // Create a Node ID to serve as the local node for the finger table
  NodeId localNodeId{"00000000-00001020-a3e1f222-928d2a31-00000000"};

  FingerTable table;

  initialiseFingerTable(table, localNodeId);

  for (int i = 0; i < 160; i++)
  {
    std::cout << table.m_fingers[i].m_interval.m_start.toString() << " - " << table.m_fingers[i].m_interval.m_end.toString() << std::endl;
  }

  for (int i = 0; i < 160; i++)
  {
    REQUIRE(table.m_fingers[i].m_interval.m_start == localNodeId + NodeId::powerOfTwo(i));
    REQUIRE(table.m_fingers[i].m_interval.m_end == localNodeId + NodeId::powerOfTwo(i + 1));
  }

  int counter = 0;
  for (const auto& intervalStart : intervalStarts)
  {
    hashing::SHA1Hash hasher;
    hashing::fromString(intervalStart.c_str(), hasher);
    REQUIRE(table.m_fingers[counter].m_interval.m_start == NodeId{hasher});
    counter++;
  }
}

}} // namespace chord::test
