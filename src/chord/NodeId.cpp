#include "NodeId.h"

#include <cstdint>
#include <cstring>
#include <sstream>
#include <bit>

namespace chord {

NodeId::NodeId()
  : m_id{0}
{
  memset(m_id, 0, 20);
}

NodeId::NodeId(const uint8_t id[20])
  : m_id{0}
{
  if (std::endian::native == std::endian::big)
  {
    memcpy(m_id, id, 20);
    return;
  }

  auto* id_p = &id[19];

  for (int i = 0; i < 20; i++)
  {
    m_id[i] = *id_p--;
  }
}

NodeId::NodeId(const hashing::SHA1Hash& hash)
  : m_id{0}
{
  if (std::endian::native == std::endian::big)
  {
    memcpy(m_id, hash, 20);
    return;
  }

  auto* hash_p = &hash[19];

  for (int i = 0; i < 20; i++)
  {
    m_id[i] = *hash_p--;
  }
}

NodeId::NodeId(const std::string& hash)
  : m_id{0}
{
  // This should only be used for testing so it doesn't matter if this code is a bit slow

  std::string hashString;
  auto it = hash.begin();

  while (it != hash.end())
  {
    if (*it == '-')
    {
      it++;
      continue;
    }

    if (!isalnum(*it)) throw;

    hashString.push_back(*it++);
  }

  if (hashString.length() != 40) throw;

  uint8_t* current_p;
  std::size_t index = 0;

  it = hashString.begin();

  while (it != hashString.end())
  {
    auto hi = *it++;
    auto lo = *it++;
    m_id[byteIndex(index)] = hexStringToByte(hi, lo);
    index++;
  }
}

NodeId::NodeId(uint32_t ipAddress)
  : m_id{0}
{

  hashing::SHA1Hash digest;
  hashing::sha1((uint8_t*) &ipAddress, 4, digest);

  if (std::endian::native == std::endian::big)
  {
    memcpy(m_id, digest, 20);
    return;
  }

  auto* digest_p = &digest[19];

  for (uint8_t& i : m_id)
  {
    i = *digest_p--;
  }
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
    result.m_id[ByteIndex<19>::value] = powerOfTwoOctet;
  }
  else if (power >=8 && power < 16)
  {
    result.m_id[ByteIndex<18>::value] = powerOfTwoOctet;
  }
  else if (power >= 16 && power < 24)
  {
    result.m_id[ByteIndex<17>::value] = powerOfTwoOctet;
  }
  else if (power >= 24 && power < 32)
  {
    result.m_id[ByteIndex<16>::value] = powerOfTwoOctet;
  }
  else if (power >=32 && power < 40)
  {
    result.m_id[ByteIndex<15>::value] = powerOfTwoOctet;
  }
  else if (power >= 40 && power < 48)
  {
    result.m_id[ByteIndex<14>::value] = powerOfTwoOctet;
  }
  else if (power >= 48 && power < 56)
  {
    result.m_id[ByteIndex<13>::value] = powerOfTwoOctet;
  }
  else if (power >= 56 && power < 64)
  {
    result.m_id[ByteIndex<12>::value] = powerOfTwoOctet;
  }
  else if (power >= 64 && power < 72)
  {
    result.m_id[ByteIndex<11>::value] = powerOfTwoOctet;
  }
  else if (power >= 72 && power < 80)
  {
    result.m_id[ByteIndex<10>::value] = powerOfTwoOctet;
  }
  else if (power >= 80 && power < 88)
  {
    result.m_id[ByteIndex<9>::value] = powerOfTwoOctet;
  }
  else if (power >= 88 && power < 96)
  {
    result.m_id[ByteIndex<8>::value] = powerOfTwoOctet;
  }
  else if (power >= 96 && power < 104)
  {
    result.m_id[ByteIndex<7>::value] = powerOfTwoOctet;
  }
  else if (power >= 104 && power < 112)
  {
    result.m_id[ByteIndex<6>::value] = powerOfTwoOctet;
  }
  else if (power >= 112 && power < 120)
  {
    result.m_id[ByteIndex<5>::value] = powerOfTwoOctet;
  }
  else if (power >= 120 && power < 128)
  {
    result.m_id[ByteIndex<4>::value] = powerOfTwoOctet;
  }
  else if (power >= 128 && power < 136)
  {
    result.m_id[ByteIndex<3>::value] = powerOfTwoOctet;
  }
  else if (power >= 136 && power < 144)
  {
    result.m_id[ByteIndex<2>::value] = powerOfTwoOctet;
  }
  else if (power >= 144 && power < 152)
  {
    result.m_id[ByteIndex<1>::value] = powerOfTwoOctet;
  }
  else if (power >= 152 && power < 160)
  {
    result.m_id[ByteIndex<0>::value] = powerOfTwoOctet;
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
  bool result = std::memcmp(m_id, other.m_id, 20) == 0;
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

  uint64_t overflow{0};

  auto* this_p = reinterpret_cast<const uint32_t*>(m_id);
  auto* other_p = reinterpret_cast<const uint32_t*>(other.m_id);
  auto* result_p = reinterpret_cast<uint32_t*>(result.m_id);

  for (size_t i = 0; i < 5; i++)
  {
    overflow += static_cast<uint64_t>(*this_p++) + static_cast<uint64_t>(*other_p++);
    *result_p++ = static_cast<uint32_t>(overflow & 0xFFFFFFFF);

    overflow = ((overflow & 0xFFFFFFFF00000000) == 0) ? 0 : 1;
  }

  return result;
}

std::string NodeId::toString() const
{
  std::stringstream ss;

  for (int i = 0; i < 20; i++)
  {
    // Each octet in the hash needs to be split into its upper and lower nybbles,
    // these need to be printed separately.
    // If the whole byte is printed then 0x00 will be printed as just 0, not 00
    ss << std::hex << (m_id[byteIndex(i)] >> 4); // Upper nybble
    ss << std::hex << (m_id[byteIndex(i)] & 0xF); // Lower nybble
  }
  return ss.str();
}

} // namespace chord

