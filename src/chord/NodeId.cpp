#include "NodeId.h"

#include <cstdint>
#include <cstring>
#include <sstream>


namespace chord {

NodeId::NodeId()
  : m_id{0},
    m_id2{0}
{
  memset(m_id, 0, 20);
}

NodeId::NodeId(const uint8_t id[20])
  : m_id{0},
    m_id2{0}
{
  memcpy(m_id, id, 20);
  memcpy(m_id2, id, 20);
}

NodeId::NodeId(const uint32_t id[5])
  : m_id{0},
    m_id2{0}
{
  memcpy(m_id, id, 20);
  memcpy(m_id2, id, 20);
}

NodeId::NodeId(const hashing::SHA1Hash& hash)
  : m_id{0},
    m_id2{0}
{
  memcpy(m_id, hash, 20);
  memcpy(m_id2, hash, 20);
}

NodeId::NodeId(const NodeId& other)
  : m_id{0},
    m_id2{0}
{
  memcpy(m_id, other.m_id, 20);
  memcpy(m_id2, other.m_id, 20);
}

NodeId NodeId::powerOfTwo(int power)
{
  NodeId result;
  if (power < 0 || power > 159)
  {
    return result;
  }

  uint8_t powerOfTwoOctet = powerOfTwoArray[power % 8];

  if (power >= 0 && power < 8)
  {
    result.m_id[19] = powerOfTwoOctet;
  }
  else if (power >=8 && power < 16)
  {
    result.m_id[18] = powerOfTwoOctet;
  }
  else if (power >= 16 && power < 24)
  {
    result.m_id[17] = powerOfTwoOctet;
  }
  else if (power >= 24 && power < 32)
  {
    result.m_id[16] = powerOfTwoOctet;
  }
  else if (power >=32 && power < 40)
  {
    result.m_id[15] = powerOfTwoOctet;
  }
  else if (power >= 40 && power < 48)
  {
    result.m_id[14] = powerOfTwoOctet;
  }
  else if (power >= 48 && power < 56)
  {
    result.m_id[13] = powerOfTwoOctet;
  }
  else if (power >= 56 && power < 64)
  {
    result.m_id[12] = powerOfTwoOctet;
  }
  else if (power >= 64 && power < 72)
  {
    result.m_id[11] = powerOfTwoOctet;
  }
  else if (power >= 72 && power < 80)
  {
    result.m_id[10] = powerOfTwoOctet;
  }
  else if (power >= 80 && power < 88)
  {
    result.m_id[9] = powerOfTwoOctet;
  }
  else if (power >= 88 && power < 96)
  {
    result.m_id[8] = powerOfTwoOctet;
  }
  else if (power >= 96 && power < 104)
  {
    result.m_id[7] = powerOfTwoOctet;
  }
  else if (power >= 104 && power < 112)
  {
    result.m_id[6] = powerOfTwoOctet;
  }
  else if (power >= 112 && power < 120)
  {
    result.m_id[5] = powerOfTwoOctet;
  }
  else if (power >= 120 && power < 128)
  {
    result.m_id[4] = powerOfTwoOctet;
  }
  else if (power >= 128 && power < 136)
  {
    result.m_id[3] = powerOfTwoOctet;
  }
  else if (power >= 136 && power < 144)
  {
    result.m_id[2] = powerOfTwoOctet;
  }
  else if (power >= 144 && power < 152)
  {
    result.m_id[1] = powerOfTwoOctet;
  }
  else if (power >= 152 && power < 160)
  {
    result.m_id[0] = powerOfTwoOctet;
  }

  return result;
}

NodeId& NodeId::operator=(const NodeId& other)
{
  memcpy(m_id, other.m_id, 20);
  return *this;
}

bool NodeId::operator==(const NodeId& other) const
{
  bool result = memcmp(m_id, other.m_id, 20) == 0;
  return result;
}

bool NodeId::operator!=(const NodeId& other) const
{
  return memcmp(m_id, other.m_id, 20) != 0;
}

bool NodeId::operator<(const NodeId& other) const
{
  return memcmp(m_id, other.m_id, 20) < 0;
}

bool NodeId::operator>(const NodeId& other) const
{
  return memcmp(m_id, other.m_id, 20) > 0;
}

bool NodeId::operator<=(const NodeId& other) const
{
  return memcmp(m_id, other.m_id, 20) <= 0;
}

bool NodeId::operator>=(const NodeId& other) const
{
  return memcmp(m_id, other.m_id, 20) >= 0;
}

NodeId NodeId::operator+(const NodeId& other) const
{
  NodeId result;

  uint64_t overflowBuffer{0};

  // This approach doesn't work exactly because it will need to take account of system endianness
  // The algorithm assumes big endian, but my system is little endian
  
  uint64_t overflowBuff0 = static_cast<uint64_t>(m_id2[0]) + static_cast<uint64_t>(other.m_id2[0]);
  uint64_t overflowBuff1 = static_cast<uint64_t>(m_id2[1]) + static_cast<uint64_t>(other.m_id2[1]);
  uint64_t overflowBuff2 = static_cast<uint64_t>(m_id2[2]) + static_cast<uint64_t>(other.m_id2[2]);
  uint64_t overflowBuff3 = static_cast<uint64_t>(m_id2[3]) + static_cast<uint64_t>(other.m_id2[3]);
  uint64_t overflowBuff4 = static_cast<uint64_t>(m_id2[4]) + static_cast<uint64_t>(other.m_id2[4]);

  result.m_id2[4] = static_cast<uint32_t>(overflowBuff4 & 0xFFFFFFFF);
  result.m_id2[3] = static_cast<uint32_t>((overflowBuff3 & 0xFFFFFFFF) + (overflowBuff4 >> 32));
  result.m_id2[2] = static_cast<uint32_t>((overflowBuff2 & 0xFFFFFFFF) + (overflowBuff3 >> 32));
  result.m_id2[1] = static_cast<uint32_t>((overflowBuff1 & 0xFFFFFFFF) + (overflowBuff2 >> 32));
  result.m_id2[0] = static_cast<uint32_t>((overflowBuff0 & 0xFFFFFFFF) + (overflowBuff1 >> 32));

  if ((overflowBuff0 >> 32) == 0) return result;

  std::cout << "Overflow buff 0 " << std::hex << overflowBuff0 << std::endl;

  overflowBuff4 = static_cast<uint64_t>(result.m_id2[4]) + (overflowBuff0 >> 32);
  result.m_id[4] = overflowBuff4 & 0xFFFFFFFF; 
  if ((overflowBuff4 >> 32) == 0 ) 
  {
    memcpy(result.m_id, result.m_id2, 20);
    std::cout << std::hex << result.m_id2[0] << std::endl;
    std::cout << std::hex << result.m_id2[1] << std::endl;
    std::cout << std::hex << result.m_id2[2] << std::endl;
    std::cout << std::hex << result.m_id2[3] << std::endl;
    std::cout << std::hex << result.m_id2[4] << std::endl;
    return result;
  }
  std::cout << "Overflow buff 4 " << std::hex << overflowBuff4 << std::endl;

  overflowBuff3 = static_cast<uint64_t>(result.m_id2[3]) + (overflowBuff4 >> 32);
  result.m_id[3] = overflowBuff3 & 0xFFFFFFFF; 
  if ((overflowBuff3 >> 32) == 0 ) return result;
  std::cout << "Overflow buff 3 " << std::hex << overflowBuff3 << std::endl;

  overflowBuff2 = static_cast<uint64_t>(result.m_id2[2]) + (overflowBuff3 >> 32);
  result.m_id[2] = overflowBuff2 & 0xFFFFFFFF; 
  if ((overflowBuff2 >> 32) == 0 ) return result;
  std::cout << "Overflow buff 2 " << std::hex << overflowBuff2 << std::endl;

  overflowBuff1 = static_cast<uint64_t>(result.m_id2[1]) + (overflowBuff2 >> 32);
  result.m_id[1] = overflowBuff1 & 0xFFFFFFFF; 
  if ((overflowBuff1 >> 32) == 0 ) return result;
  std::cout << "Overflow buff 1 " << std::hex << overflowBuff1 << std::endl;

  overflowBuff0 = static_cast<uint64_t>(m_id2[0]) + static_cast<uint64_t>(other.m_id2[3]);
  result.m_id[0] = overflowBuff0 & 0xFFFFFFFF; 
  std::cout << "Overflow buff 0 " << std::hex << overflowBuff0 << std::endl;

  memcpy(result.m_id, result.m_id2, 20);

  return result;
  
  //for (int i = 4; i > 0; i--)
  //{
  //  // first convert the two bytes to a 16 bit integer, this will overflow but that is ok because we will add the overflow to the next byte.
  //  uint64_t sum = static_cast<uint64_t>(m_id2[i]) + static_cast<uint64_t>(other.m_id2[i]) + overflowBuffer;

  //  // mask off the lower 8 bits and add them to the result.
  //  result.m_id[i] += static_cast<uint32_t>(sum & 0xFFFFFFFF);

  //  // shift the upper 8 bits to the lower 8 bits and add them to the next byte.
  //  // However we need to do this with the overflow buffer
  //  //result.m_id[i - 1] += static_cast<uint8_t>(sum >> 8);
  //  overflowBuffer = sum >> 32;
  //}

  //// deal with most significant octet
  //uint64_t sum = static_cast<uint64_t>(m_id2[0]) + static_cast<uint64_t>(other.m_id2[0]) + overflowBuffer;

  //result.m_id2[0] = sum & 0xFFFFFFFF;
  //
  //if ((sum >> 32) == 0) return result;

  //overflowBuffer = sum >> 8;

  //for (int i = 5; i > 0; i--)
  //{
  //  sum = result.m_id2[i] + overflowBuffer;

  //  overflowBuffer = sum >> 32;

  //  if (overflowBuffer == 0) return result;
  //}

  //return result;
}

std::string NodeId::toString() const
{
  std::stringstream ss;
  int counter{0};
  
  for (unsigned char i : m_id)
  {
    // Each octet in the hash needs to be split into its upper and lower nybbles, 
    // these need to be printed separately.
    // If the whole byte is printed then 0x00 will be printed as just 0, not 00
    ss << std::hex << (i >> 4); // Upper nybble
    ss << std::hex << (i & 0xF); // Lower nybble
  }
  return ss.str();
}

} // namespace chord

