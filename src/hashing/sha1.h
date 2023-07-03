#ifndef SHA1_H_
#define SHA1_H_

#include <cstdint>
#include <cstring>
#include <climits>
#include <iostream>
#include <sys/types.h>

namespace hashing {

static constexpr uint32_t H0 = 0x67452301;
static constexpr uint32_t H1 = 0xEFCDAB89;
static constexpr uint32_t H2 = 0x98BADCFE;
static constexpr uint32_t H3 = 0x10325476;
static constexpr uint32_t H4 = 0xC3D2E1F0;

using SHA1Hash = uint8_t[20];

void sha1(const uint8_t *message, uint32_t messageLength, uint8_t *digest);

void sha1ToHex(const uint8_t* digest, char* hexDigest);

void fromString(const char* hexDigest, uint8_t* digest);

inline uint32_t numberOfPaddingBits(uint32_t messageLength)
{
  uint64_t messageLengthInBits = messageLength * 8;

  // work out the number of padding bits that we need in order to make the message length 
  uint32_t numberOfPaddingBits = (448 - messageLengthInBits) % 512;

  return numberOfPaddingBits;
}

inline bool isLittleEndian()
{
  uint32_t test = 1;
  auto* test_p = reinterpret_cast<uint8_t*>(&test);

  if (*test_p == 1) 
  {
    return true;
  }

  return false;
}

inline uint16_t swapEndianU16(uint16_t value)
{
  return (value << 8) | (value >> 8);
}

inline uint32_t swapEndianU32(uint32_t value)
{
  value = ((value << 8) & 0xFF00FF00) | ((value >> 8) & 0xFF00FF);
  return (value << 16) | (value >> 16);
}

inline uint64_t swapEndianU64(uint64_t value)
{
  value = ((value << 8) & 0xFF00FF00FF00FF00ULL) | ((value >> 8) & 0x00FF00FF00FF00FFULL);
  value = ((value << 16) & 0xFFFF0000FFFF0000ULL) | ((value >> 16) & 0x0000FFFF0000FFFFULL);
  return (value << 32) | (value >> 32);
}

// This function allocates memory that must be deallocated by the caller
// If you call this function you must take responsibility for that memory
inline uint8_t* padMessage(const uint8_t *message, uint32_t messageLength)
{
  uint32_t paddingLengthInBits = numberOfPaddingBits(messageLength);
  uint32_t paddingLengthInBytes = paddingLengthInBits / 8;

  // The length of the padded message is the sum of:
  //   - the length of the original message,
  //   - the length of the 10..00 padding,
  //   - the 8 bytes of a 64-bit integer that is the original message length in bits.
  uint8_t* paddedMessage = new uint8_t[messageLength + paddingLengthInBytes + 8]; 

  memcpy(paddedMessage, message, messageLength);

  uint8_t* paddedMessagePosition_p = paddedMessage + messageLength;

  // Now add the padding:
  *paddedMessagePosition_p++ = uint8_t{ 0x80 };

  for (int i = 1; i < paddingLengthInBytes; i++)
  {
    *paddedMessagePosition_p++ = uint8_t{ 0 };
  }

  uint64_t messageLengthInBits = messageLength * 8;

  if (isLittleEndian())
  {
    messageLengthInBits = swapEndianU64(messageLengthInBits);
  }

  memcpy(paddedMessagePosition_p, &messageLengthInBits, 8);

  return paddedMessage;
}

inline uint32_t rotateLeft(uint32_t value, unsigned int distance)
{
  return (value << distance) | (value >> (32 - distance));
}

enum class ShaFunction
{
  F1,
  F2,
  F3,
  F4,
};

template<ShaFunction shaFunction>
inline uint32_t F(uint32_t B, uint32_t C, uint32_t D)
{
  if constexpr (shaFunction == ShaFunction::F1)
  {
    return (B & C) | (~B & D);
  }
  else if constexpr (shaFunction == ShaFunction::F2)
  {
    return B ^ C ^ D;
  }
  else if constexpr (shaFunction == ShaFunction::F3)
  {
    return (B & C) | (B & D) | (C & D);
  }
  else if constexpr (shaFunction == ShaFunction::F4)
  {
    return B ^ C ^ D;
  }
}

template<ShaFunction shaFunction>
inline uint32_t K()
{
  if constexpr (shaFunction == ShaFunction::F1)
  {
    return 0x5A827999;
  }
  else if constexpr (shaFunction == ShaFunction::F2)
  {
    return 0x6ED9EBA1;
  }
  else if constexpr (shaFunction == ShaFunction::F3)
  {
    return 0x8F1BBCDC;
  }
  else if constexpr (shaFunction == ShaFunction::F4)
  {
    return 0xCA62C1D6;
  }
}

template<ShaFunction shaFunction>
inline void sha1Round(uint32_t& A, uint32_t& B, uint32_t& C, uint32_t& D, uint32_t& E, uint32_t W)
{
  uint32_t temp = rotateLeft(A, 5) + F<shaFunction>(B, C, D) + E + W + K<shaFunction>();
  E = D;
  D = C;
  C = rotateLeft(B, 30);
  B = A;
  A = temp;
}

} // namespace hashing

#endif // SHA1_H_
