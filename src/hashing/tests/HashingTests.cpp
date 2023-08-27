#include <catch2/catch_test_macros.hpp>

#include <cstdint>
#include <cstring>
#include <iostream>

#include "../sha1.h"

namespace odd::hashing::test {

TEST_CASE("String")
{
  std::string sha1 = "a9993e364706816aba3e25717850c26c9cd0d89d";
  uint8_t hash[20];
  fromString(sha1.c_str(), hash);

  REQUIRE(hash[0] == 0xa9);
  REQUIRE(hash[1] == 0x99);
  REQUIRE(hash[2] == 0x3e);
  REQUIRE(hash[3] == 0x36);
  REQUIRE(hash[4] == 0x47);
  REQUIRE(hash[5] == 0x06);
  REQUIRE(hash[6] == 0x81);
  REQUIRE(hash[7] == 0x6a);
  REQUIRE(hash[8] == 0xba);
  REQUIRE(hash[9] == 0x3e);
  REQUIRE(hash[10] == 0x25);
  REQUIRE(hash[11] == 0x71);
  REQUIRE(hash[12] == 0x78);
  REQUIRE(hash[13] == 0x50);
  REQUIRE(hash[14] == 0xc2);
  REQUIRE(hash[15] == 0x6c);
  REQUIRE(hash[16] == 0x9c);
  REQUIRE(hash[17] == 0xd0);
  REQUIRE(hash[18] == 0xd8);
  REQUIRE(hash[19] == 0x9d);
}

TEST_CASE("Padding length can be correctly calculated")
{
  // Create a message of ten bytes this 80 bits
  uint8_t message[10];

  // The SHA algorith requires a message length that is a multiple of 512 in bits
  // Therefore, to hash this message, we are going to need to pad it until it is 512 bits
  
  uint32_t paddingBits = numberOfPaddingBits(10);
  uint32_t expectedPaddingBits = 512 - 80 - 64;
 
  REQUIRE((paddingBits % 8) == 0);

  REQUIRE(paddingBits == expectedPaddingBits);
}

TEST_CASE("Test the padding of a message")
{
  uint8_t message[20];
  message[0] = 1; message[1] = 2; message[2] = 3; message[3] = 4; message[4] = 5; message[5] = 6;
  message[6] = 7; message[7] = 8; message[8] = 9; message[9] = 10; message[10] = 11; message[11] = 12;
  message[12] = 13; message[13] = 14; message[14] = 15; message[15] = 16; message[16] = 17; message[17] = 18;
  message[18] = 19; message[19] = 20;

  uint8_t* paddedMessage = padMessage(message, 20);

  for (int i = 0; i < 20; i++)
  {
    REQUIRE(paddedMessage[i] == message[i]);
  }

  REQUIRE(paddedMessage[20] == 0x80);

  for (int i = 21; i < 63; i++)
  {
    REQUIRE(paddedMessage[i] == 0x00);
  }

  REQUIRE(paddedMessage[63] == 0xA0);
}

TEST_CASE("Hash a string and see what happens")
{
  std::string stringToHash = "abc";

  uint8_t digest[20];
  uint8_t expectedDigest[20] = {0xa9, 0x99, 0x3e, 0x36, 0x47, 0x06, 0x81, 0x6a, 0xba, 0x3e, 0x25, 0x71, 0x78, 0x50, 0xc2, 0x6c, 0x9c, 0xd0, 0xd8, 0x9d};

  sha1((uint8_t*) stringToHash.c_str(), stringToHash.length(), digest);

  for (int i = 0; i < 20; i++)
  {
    REQUIRE(digest[i] == expectedDigest[i]);
  }
} 

TEST_CASE("Hash a sentence")
{
  std::string stringToHash = "The quick brown fox jumps over the lazy dog";

  uint8_t digest[20];
  uint8_t expectedDigest[20] = {0x2f, 0xd4, 0xe1, 0xc6, 0x7a, 0x2d, 0x28, 0xfc, 0xed, 0x84, 0x9e, 0xe1, 0xbb, 0x76, 0xe7, 0x39, 0x1b, 0x93, 0xeb, 0x12};

  sha1((uint8_t*) stringToHash.c_str(), stringToHash.length(), digest);

  for (int i = 0; i < 20; i++)
  {
    REQUIRE(digest[i] == expectedDigest[i]);
  }
}

} // namespace odd::hashing::test
