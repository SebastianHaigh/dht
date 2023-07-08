#include "Comms.h"
#include "CommsCoder.h"

#include <bit>
#include <cstdint>
#include <type_traits>

EncodedMessage::EncodedMessage(std::size_t requiredLength)
  : m_message(new uint8_t[requiredLength]),
    m_length(requiredLength)
{
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

Message::Message(CommsVersion version, MessageType type, std::size_t payloadLength)
  : m_version(version),
    m_type(type),
    m_payloadLength(payloadLength)
{
}

Message::Message(CommsVersion version, MessageType type)
  : m_version(version),
    m_type(type),
    m_payloadLength(0)
{
}

[[nodiscard]] const MessageType& Message::type() const
{
  return m_type;
}

[[nodiscard]] const CommsVersion& Message::version() const
{
  return m_version;
}

[[nodiscard]] std::size_t Message::payloadLength() const
{
  return m_payloadLength;
}

EncodedMessage Message::createEncodedMessage()
{
  std::size_t messageLength = m_payloadLength + 28;

  EncodedMessage encoded{messageLength};

  auto* msg_p = encoded.m_message;

  encodeSingleValue((uint16_t*) &m_version, msg_p);
  encodeSingleValue((uint32_t*) &m_type, msg_p + sizeof(CommsVersion));
  encodeSingleValue((uint16_t*) &m_payloadLength, msg_p + sizeof(CommsVersion) + sizeof(MessageType));

  return std::move(encoded);
}

void Message::decodeHeaders(const EncodedMessage& encodedMessage)
{
  auto* msg_p = encodedMessage.m_message;

  decodeSingleValue(msg_p, (uint16_t*) &m_version);
  decodeSingleValue(msg_p + sizeof(CommsVersion), (uint32_t*) &m_type);
  decodeSingleValue(msg_p + sizeof(CommsVersion) + sizeof(MessageType), (uint16_t*) &m_payloadLength);
}

// Should the checksum be part of this message or should it be in the wrapped packet that is generated for point to point cast
void Message::generateChecksum(EncodedMessage& encodedMessage)
{
  std::size_t checksumStartIndex{encodedMessage.m_length - 20};

  auto* checksum_p = &encodedMessage.m_message[checksumStartIndex];

  hashing::sha1(encodedMessage.m_message, checksumStartIndex, checksum_p);
}

JoinMessage::JoinMessage(CommsVersion version)
  : Message{version,
            MessageType::JOIN,
            4},
    m_ip(0)
{
}

JoinMessage::JoinMessage(CommsVersion version, uint32_t ip)
  : Message{version,
            MessageType::JOIN,
            4},
    m_ip(ip)
{
}

EncodedMessage JoinMessage::encode()
{
  auto encoded = createEncodedMessage();

  auto* payload_p = &encoded.m_message[2 + 4 + 2];

  encodeSingleValue(&m_ip, payload_p);

  generateChecksum(encoded);

  return std::move(encoded);
}

void JoinMessage::decode(const EncodedMessage& message)
{
  decodeHeaders(message);

  auto* payload_p = &message.m_message[8];

  decodeSingleValue(payload_p, &m_ip);
}

[[nodiscard]] uint32_t JoinMessage::ip() const
{
  return m_ip;
}