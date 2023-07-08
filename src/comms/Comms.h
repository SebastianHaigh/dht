#ifndef COMMS_H_
#define COMMS_H_

#include <cassert>
#include "../hashing/sha1.h"

// This file defines the way in which messages are passed between nodes
//
// We need to be able to send abitrary length messages, but we don't want the overhead to be too large
//
// we can have a basic system of:
//
// message-type
// message-length
// message-content
// footer
//
// All messages are send big endian so this will have to be converted if the system is little endian
//
// Messaging-Version 2 bytes
// Message-Type 4 bytes
// payload-length 2 bytes
// payload (payload-length bytes)
// footer 20 bytes
//
// the footer is an sha1 hash that checks the consistancy of the message

// The messaging protocol will work like this:
//
// Step 1: endian convert and read the messaging version, if unsupported drop
// Step 2: endian convert and read the message type
// Step 3: endian convert and read the payload-length
// Step 4: use the payload length the work out the position of the footer and read
// Step 5: verify the message and hash match
// Step 6: create an object matching the message type and use this to decode the message

#include <cstddef>
#include <cstdint>
#define COMMS_VERSION 1

enum class CommsVersion : uint16_t
{
  V1 = 0x0001,
};

enum class MessageType : uint32_t
{
  JOIN              = 0x00000001,
  JOIN_RESPONSE     = 0x00000002,
};

inline std::size_t minimumMessageLength()
{
  return 28;
}

inline uint8_t* getMessageType_p(uint8_t* message)
{
  return message + 2;
}

inline uint8_t* getPayloadLength_p(uint8_t* message)
{
  return message + 6;
}

inline uint8_t* getPayload_p(uint8_t* message)
{
  return message + 2;
}

inline uint8_t* getChecksum_p(uint8_t* message, std::size_t payloadLength)
{
  return getPayload_p(message) + payloadLength;
}

bool isVersionSupported(uint8_t* message);

inline MessageType getMessageType(uint8_t* message)
{
  auto* messageType_p = getMessageType_p(message);
  auto* messageTypeUint32_p = reinterpret_cast<uint32_t*>(messageType_p);

  return static_cast<MessageType>(*messageTypeUint32_p);
}

inline uint16_t getPayloadLength(uint8_t* message)
{
  auto* payloadLength_p = getPayloadLength_p(message);
  auto* payloadLengthUint16_t = reinterpret_cast<uint16_t*>(payloadLength_p);

  return *payloadLengthUint16_t;
}

inline std::size_t getMessageLengthWithoutChecksum(uint8_t* message)
{
  return 2 + 4 + 2 + getPayloadLength(message);
}

inline bool verifyChecksum(uint8_t* message)
{
  auto* checksum_p = getChecksum_p(message, getPayloadLength(message));

  hashing::SHA1Hash messageHash;

  hashing::sha1(message, getMessageLengthWithoutChecksum(message), messageHash);

  for (uint8_t i : messageHash)
  {
    if (*checksum_p != i) return false;

    checksum_p++;
  }

  return true;
}

class EncodedMessage
{
  public:
    explicit EncodedMessage(std::size_t requiredLength);
    ~EncodedMessage();

    EncodedMessage(const EncodedMessage&) = delete;
    EncodedMessage& operator=(const EncodedMessage&) = delete;

    EncodedMessage(EncodedMessage&& rhs) noexcept;
    EncodedMessage& operator=(EncodedMessage&& rhs) noexcept;

    uint8_t* m_message;
    std::size_t m_length;
};

class Message
{
  public:
    Message(CommsVersion version, MessageType type, std::size_t payloadLength);
    Message(CommsVersion version, MessageType type);
    virtual ~Message() = default;

    [[nodiscard]] const MessageType& type() const;
    [[nodiscard]] const CommsVersion& version() const;
    [[nodiscard]] std::size_t payloadLength() const;

    virtual EncodedMessage encode() = 0;
    virtual void decode(const EncodedMessage& message) = 0;
  
  protected:
    EncodedMessage createEncodedMessage();

    void decodeHeaders(const EncodedMessage& encodedMessage);

    void generateChecksum(EncodedMessage& encodedMessage);

  private:
    CommsVersion m_version;
    MessageType m_type;
    std::size_t m_payloadLength;
};

class JoinMessage : public Message
{
  public:
    explicit JoinMessage(CommsVersion version);
    JoinMessage(CommsVersion version, uint32_t ip);
    ~JoinMessage() = default;

    EncodedMessage encode() override;
    void decode(const EncodedMessage& message) override;

    [[nodiscard]] uint32_t ip() const;

  private:

    uint32_t m_ip;
};

#endif // COMMS_H_