#include <catch2/catch_test_macros.hpp>
#include "../Messages.h"

TEST_CASE("Encode and decode a position message")
{
  NodeId nodeId{1};
  std::vector<NodeId> neighbours{ 2, 3, 4, 5, 6 };
  PositionMessage message{ nodeId, neighbours };

  uint8_t buffer[49];
  size_t bufferSize = 49;

  message.encode(buffer, bufferSize);

  PositionMessage decodedMessage{};

  decodedMessage.decode(buffer, bufferSize);

  REQUIRE(decodedMessage.m_id == nodeId);

  for (int i = 0; i < 5; i++)
  {
    REQUIRE(decodedMessage.m_neighbours[i] == neighbours[i]);
  }
}


