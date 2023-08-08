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

EncodedMessage::EncodedMessage(uint8_t* message, std::size_t messageLength)
  : m_message(new uint8_t[messageLength]),
    m_length(messageLength)
{
  memcpy(m_message, message, m_length);
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

EncodedMessage Message::createEncodedMessage() const
{
  std::size_t messageLength = m_payloadLength + 8;

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

JoinMessage::JoinMessage(CommsVersion version)
  : Message{version,
            MessageType::JOIN,
            8},
    m_ip(0),
    m_requestId(0)
{
}

JoinMessage::JoinMessage(CommsVersion version, uint32_t ip, uint32_t requestId)
  : Message{version,
            MessageType::JOIN,
            8},
    m_ip(ip),
    m_requestId(requestId)
{
}

EncodedMessage JoinMessage::encode() const
{
  auto encoded = createEncodedMessage();

  auto* payload_p = &encoded.m_message[2 + 4 + 2];

  encodeSingleValue(&m_ip, payload_p);
  payload_p += sizeof(uint32_t);

  encodeSingleValue(&m_requestId, payload_p);

  return std::move(encoded);
}

void JoinMessage::decode(EncodedMessage&& message)
{
  decodeHeaders(message);

  auto* payload_p = &message.m_message[8];

  decodeSingleValue(payload_p, &m_ip);
  payload_p += sizeof(uint32_t);

  decodeSingleValue(payload_p, &m_requestId);
}

[[nodiscard]] uint32_t JoinMessage::ip() const
{
  return m_ip;
}

[[nodiscard]] uint32_t JoinMessage::requestId() const
{
  return m_requestId;
}

JoinResponseMessage::JoinResponseMessage(CommsVersion version)
  : Message{version,
            MessageType::JOIN_RESPONSE,
            8},
    m_ip(0),
    m_requestId(0)
{
}

JoinResponseMessage::JoinResponseMessage(CommsVersion version, uint32_t ip, uint32_t requestId)
  : Message{version,
            MessageType::JOIN_RESPONSE,
            8},
    m_ip(ip),
    m_requestId(requestId)
{
}

EncodedMessage JoinResponseMessage::encode() const
{
  auto encoded = createEncodedMessage();

  auto* payload_p = &encoded.m_message[2 + 4 + 2];

  encodeSingleValue(&m_ip, payload_p);
  payload_p += sizeof(uint32_t);

  encodeSingleValue(&m_requestId, payload_p);

  return std::move(encoded);
}

void JoinResponseMessage::decode(EncodedMessage&& message)
{
  decodeHeaders(message);

  auto* payload_p = &message.m_message[8];

  decodeSingleValue(payload_p, &m_ip);
  payload_p += sizeof(uint32_t);

  decodeSingleValue(payload_p, &m_requestId);
}

[[nodiscard]] uint32_t JoinResponseMessage::ip() const
{
  return m_ip;
}

[[nodiscard]] uint32_t JoinResponseMessage::requestId() const
{
  return m_requestId;
}

PositionMessage::PositionMessage(CommsVersion version)
  : Message{version,
            MessageType::JOIN_RESPONSE,
            4},
    m_ip(0)
{
}

PositionMessage::PositionMessage(CommsVersion version, uint32_t ip)
  : Message{version,
            MessageType::JOIN_RESPONSE,
            4},
    m_ip(ip)
{
}

EncodedMessage PositionMessage::encode() const
{
  auto encoded = createEncodedMessage();

  auto* payload_p = &encoded.m_message[2 + 4 + 2];

  encodeSingleValue(&m_ip, payload_p);

  return std::move(encoded);
}

void PositionMessage::decode(EncodedMessage&& message)
{
  decodeHeaders(message);

  auto* payload_p = &message.m_message[8];

  decodeSingleValue(payload_p, &m_ip);
}

[[nodiscard]] uint32_t PositionMessage::ip() const
{
  return m_ip;
}
