#include "NodeId.h"

#include <cstring>
#include <sstream>


namespace chord {

NodeId::NodeId()
  : m_id{0}
{
  memset(m_id, 0, 20);
}

NodeId::NodeId(const uint8_t id[20])
  : m_id{0}
{
  memcpy(m_id, id, 20);
}

NodeId::NodeId(const hashing::SHA1Hash& hash)
  : m_id{0}
{
  memcpy(m_id, hash, 20);
}

NodeId::NodeId(const NodeId& other)
  : m_id{0}
{
  memcpy(m_id, other.m_id, 20);
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
  std::cout << "NodeId::operator==()" << std::endl;
  bool result = memcmp(m_id, other.m_id, 20) == 0;
  std::cout << "NodeId::operator==() - result = " << result << std::endl;
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

  for (int i = 20; i > 0; i--)
  {
    // first convert the two bytes to a 16 bit integer, this will overflow but that is ok because we will add the overflow to the next byte.
    uint16_t sum = m_id[i] + other.m_id[i];

    // mask off the lower 8 bits and add them to the result.
    result.m_id[i] = sum & 0xFF;

    // shift the upper 8 bits to the lower 8 bits and add them to the next byte.
    result.m_id[i - 1] = sum >> 8;
  }

  return result;
}

std::string NodeId::toString() const
{
  std::stringstream ss;
  for (int i = 0; i < 20; i++)
  {
    ss << std::hex << static_cast<int>(m_id[i]);
  }
  return ss.str();
}

} // namespace chord

