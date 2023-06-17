#ifndef NODE_ID_H_
#define NODE_ID_H_

#include "../hashing/sha1.h"
#include <array>
#include <cstdint>
#include <cmath>

namespace chord {

static constexpr uint8_t powerOfTwoArray[8] = { 1, 2, 4, 8, 16, 32, 64, 128 };

struct NodeId
{
  NodeId();
  explicit NodeId(const uint8_t id[20]);
  explicit NodeId(const hashing::SHA1Hash& hash);

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

  static NodeId powerOfTwo(int power);

  hashing::SHA1Hash m_id;
};

}

#endif // NODE_ID_H_
