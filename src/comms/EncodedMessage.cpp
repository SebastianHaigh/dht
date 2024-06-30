#include "EncodedMessage.h"

#include <cstring>

namespace odd {

EncodedMessage::EncodedMessage(std::size_t requiredLength)
  : m_message(new uint8_t[requiredLength]),
    m_length(requiredLength)
{
}

EncodedMessage::EncodedMessage(uint8_t* message, std::size_t messageLength)
  : m_message(new uint8_t[messageLength]),
    m_length(messageLength)
{
  std::memcpy(m_message, message, m_length);
}

EncodedMessage::~EncodedMessage()
{
  delete[] m_message;
}

EncodedMessage::EncodedMessage(EncodedMessage&& rhs) noexcept
  : m_message(rhs.m_message),
    m_length(rhs.m_length)
{
  rhs.m_message = nullptr;
}

EncodedMessage& EncodedMessage::operator=(EncodedMessage&& rhs) noexcept
{
  m_message = rhs.m_message;
  m_length = rhs.m_length;
  rhs.m_message = nullptr;

  return *this;
}

} // namespace odd
