#ifndef COMMS_ENCODED_MESSAGE_H_
#define COMMS_ENCODED_MESSAGE_H_

#include <cstddef>
#include <cstdint>
#include <vector>

namespace odd {

class EncodedMessage
{
public:
  explicit EncodedMessage(std::size_t requiredLength);
  EncodedMessage(const uint8_t* message, std::size_t messageLength);

  [[nodiscard]] EncodedMessage clone() const;

  uint8_t* message() { return m_message.data(); }
  [[nodiscard]] const uint8_t* message() const { return m_message.data(); }
  [[nodiscard]] std::size_t length() const { return m_message.size(); }

private:
  std::vector<uint8_t> m_message;
};

} // namespace odd

#endif // COMMS_ENCODED_MESSAGE_H_

