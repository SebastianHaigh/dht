#ifndef COMMS_ENCODED_MESSAGE_H_
#define COMMS_ENCODED_MESSAGE_H_

#include <cstddef>
#include <cstdint>

namespace odd {

class EncodedMessage
{
public:
  explicit EncodedMessage(std::size_t requiredLength);
  EncodedMessage(const uint8_t* message, std::size_t messageLength);
  ~EncodedMessage();

  EncodedMessage(const EncodedMessage&) = delete;
  EncodedMessage& operator=(const EncodedMessage&) = delete;

  EncodedMessage(EncodedMessage&& rhs) noexcept;
  EncodedMessage& operator=(EncodedMessage&& rhs) noexcept;

  [[nodiscard]] EncodedMessage clone() const;

  uint8_t* message() { return m_message; }
  [[nodiscard]] const uint8_t* message() const { return m_message; }
  [[nodiscard]] std::size_t length() const { return m_length; }

private:
  uint8_t* m_message;
  std::size_t m_length;
};

} // namespace odd

#endif // COMMS_ENCODED_MESSAGE_H_

