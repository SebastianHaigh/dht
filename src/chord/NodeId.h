#ifndef NODE_ID_H_
#define NODE_ID_H_

#include "../hashing/sha1.h"
#include <array>
#include <cstdint>
#include <cmath>
#include <bit>

namespace chord {

static constexpr uint8_t powerOfTwoArray[8] = { 1, 2, 4, 8, 16, 32, 64, 128 };

template<std::size_t N>
struct IndexSwap
{
  static constexpr std::size_t value = (N == 19) ? 0 :
                                       (N == 18) ? 1 :
                                       (N == 17) ? 2 :
                                       (N == 16) ? 3 :
                                       (N == 15) ? 4 :
                                       (N == 14) ? 5 :
                                       (N == 13) ? 6 :
                                       (N == 12) ? 7 :
                                       (N == 11) ? 8 :
                                       (N == 10) ? 9 :
                                       (N == 9) ? 10 :
                                       (N == 8) ? 11 :
                                       (N == 7) ? 12 :
                                       (N == 6) ? 13 :
                                       (N == 5) ? 14 :
                                       (N == 4) ? 15 :
                                       (N == 3) ? 16 :
                                       (N == 2) ? 17 :
                                       (N == 1) ? 18 :
                                       (N == 0) ? 19 : N;

};

template<std::size_t N>
struct ByteIndex
{
  static constexpr std::size_t value = (std::endian::native == std::endian::little) ? IndexSwap<N>::value : N;
};

inline std::size_t byteIndex(std::size_t index)
{
  switch (index)
  {
    case 0: return ByteIndex<0>::value;
    case 1: return ByteIndex<1>::value;
    case 2: return ByteIndex<2>::value;
    case 3: return ByteIndex<3>::value;
    case 4: return ByteIndex<4>::value;
    case 5: return ByteIndex<5>::value;
    case 6: return ByteIndex<6>::value;
    case 7: return ByteIndex<7>::value;
    case 8: return ByteIndex<8>::value;
    case 9: return ByteIndex<9>::value;
    case 10: return ByteIndex<10>::value;
    case 11: return ByteIndex<11>::value;
    case 12: return ByteIndex<12>::value;
    case 13: return ByteIndex<13>::value;
    case 14: return ByteIndex<14>::value;
    case 15: return ByteIndex<15>::value;
    case 16: return ByteIndex<16>::value;
    case 17: return ByteIndex<17>::value;
    case 18: return ByteIndex<18>::value;
    case 19: return ByteIndex<19>::value;
    default: return index;
  }
}

inline uint8_t hexCharToLowNybble(char hex)
{
  switch (hex)
  {
    case '0': return 0x00;
    case '1': return 0x01;
    case '2': return 0x02;
    case '3': return 0x03;
    case '4': return 0x04;
    case '5': return 0x05;
    case '6': return 0x06;
    case '7': return 0x07;
    case '8': return 0x08;
    case '9': return 0x09;
    case 'A':
    case 'a': return 0x0A;
    case 'B':
    case 'b': return 0x0B;
    case 'C':
    case 'c': return 0x0C;
    case 'D':
    case 'd': return 0x0D;
    case 'E':
    case 'e': return 0x0E;
    case 'F':
    case 'f': return 0x0F;
    default: return 0xFF;
  }
}

inline uint8_t hexCharToHighNybble(char hex)
{
  switch (hex)
  {
    case '0': return 0x00;
    case '1': return 0x10;
    case '2': return 0x20;
    case '3': return 0x30;
    case '4': return 0x40;
    case '5': return 0x50;
    case '6': return 0x60;
    case '7': return 0x70;
    case '8': return 0x80;
    case '9': return 0x90;
    case 'A':
    case 'a': return 0xA0;
    case 'B':
    case 'b': return 0xB0;
    case 'C':
    case 'c': return 0xC0;
    case 'D':
    case 'd': return 0xD0;
    case 'E':
    case 'e': return 0xE0;
    case 'F':
    case 'f': return 0xF0;
    default: return 0xFF;
  }
}

inline uint8_t hexStringToByte(char high, char low)
{
  auto hi = hexCharToHighNybble(high);
  auto lo = hexCharToLowNybble(low);

  if (hi == 0xFF || lo == 0xFF) throw;

  return (hi | lo);
}


struct NodeId
{
  NodeId();
  explicit NodeId(const uint8_t id[20]);
  explicit NodeId(const hashing::SHA1Hash& hash);
  explicit NodeId(const std::string& hash);
  explicit NodeId(uint32_t ipAddress);

  NodeId(const NodeId& other);
  NodeId& operator=(const NodeId& other);

  bool operator==(const NodeId& other) const;
  bool operator!=(const NodeId& other) const;

  bool operator<(const NodeId& other) const;
  bool operator>(const NodeId& other) const;

  bool operator<=(const NodeId& other) const;
  bool operator>=(const NodeId& other) const;

  NodeId operator+(const NodeId& other) const;

  std::string toString() const;

  const uint8_t* data() const
  {
    return &m_id[0];
  }

  static NodeId powerOfTwo(int power);

  hashing::SHA1Hash m_id;
};

bool intervalWrapsZero(const NodeId& begin, const NodeId& end);

bool containedInClosedInterval(const NodeId& begin, const NodeId& end, const NodeId& value);
bool containedInOpenInterval(const NodeId& begin, const NodeId& end, const NodeId& value);
bool containedInLeftOpenInterval(const NodeId& begin, const NodeId& end, const NodeId& value);
bool containedInRightInterval(const NodeId& begin, const NodeId& end, const NodeId& value);

}

#endif // NODE_ID_H_
