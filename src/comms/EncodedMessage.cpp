#include "EncodedMessage.h"

#include <cstring>

namespace odd {

EncodedMessage::EncodedMessage(std::size_t length)
  : m_message(length)
{
}

EncodedMessage::EncodedMessage(const uint8_t* message, std::size_t length)
  : m_message(message, message + length)
{
}

EncodedMessage EncodedMessage::clone() const
{
  return *this;
}

} // namespace odd
