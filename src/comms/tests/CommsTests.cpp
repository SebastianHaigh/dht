#include <catch2/catch_test_macros.hpp>
#include "../Comms.h"

TEST_CASE("Join Message can be encoded and decoded")
{
  JoinMessage message{ CommsVersion::V1, 0x67000001 };

  EncodedMessage encoded = message.encode();

  auto* msg_p = encoded.m_message;

  // VERSION
  CHECK(*msg_p++ == 0x00);
  CHECK(*msg_p++ == 0x01);

  // MESSAGE TYPE
  CHECK(*msg_p++ == 0x00);
  CHECK(*msg_p++ == 0x00);
  CHECK(*msg_p++ == 0x00);
  CHECK(*msg_p++ == 0x01);

  // PAYLOAD LENGTH
  CHECK(*msg_p++ == 0x00);
  CHECK(*msg_p++ == 0x04);

  // PAYLOAD
  CHECK(*msg_p++ == 0x67);
  CHECK(*msg_p++ == 0x00);
  CHECK(*msg_p++ == 0x00);
  CHECK(*msg_p++ == 0x01);

  JoinMessage decoded{ CommsVersion::V1 };

  decoded.decode(encoded);

  CHECK(decoded.version() == CommsVersion::V1);
  CHECK(decoded.type() == MessageType::JOIN);
  CHECK(decoded.payloadLength() == 4);

  CHECK(decoded.ip() == 0x67000001);
}

