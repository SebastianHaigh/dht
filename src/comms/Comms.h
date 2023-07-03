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
//
// All messages are send big endian so this will have to be converted if the system is little endian
//
// Messaging-Version 2 bytes
// Message-Type 4 bytes
// payload-length 2 bytes
// payload (payload-length bytes)
//
// The messaging protocol will work like this:
//
// Step 1: endian convert and read the messaging version, if unsupported drop
// Step 2: endian convert and read the message type
// Step 3: create an object matching the message type and use this to decode the message
// Step 4: endian convert and read the payload-length
// Step 5: decode the payload

#include <cstddef>
#include <cstdint>
#define COMMS_VERSION 1

enum class CommsVersion : uint16_t
{
  V1 = 0x0001,
};

enum class MessageType : uint32_t
{
  JOIN                           = 0x00000001,
  JOIN_RESPONSE                  = 0x00000002,

  POSITION                       = 0x00000101,

  CHORD_FIND_SUCCESSOR           = 0x00000201,
  CHORD_CLOSEST_PRECEDING_NODE   = 0x00000202,
  CHORD_STABILISE                = 0x00000203,
  CHORD_NOTIFY                   = 0x00000204,
  CHORD_FIX_FINGERS              = 0x00000205,
  CHORD_CHECK_PREDECESSOR        = 0x00000206,
};

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

class JoinResponseMessage : public Message
{
  public:
    explicit JoinResponseMessage(CommsVersion version);
    JoinResponseMessage(CommsVersion version, uint32_t ip);
    ~JoinResponseMessage() = default;

    EncodedMessage encode() override;
    void decode(const EncodedMessage& message) override;

    [[nodiscard]] uint32_t ip() const;

  private:

    uint32_t m_ip;
};

class PositionMessage : public Message
{
  public:
    explicit PositionMessage(CommsVersion version);
    PositionMessage(CommsVersion version, uint32_t ip);
    ~PositionMessage() = default;

    EncodedMessage encode() override;
    void decode(const EncodedMessage& message) override;

    [[nodiscard]] uint32_t ip() const;
    [[nodiscard]] std::size_t numNeighbours() const;
    [[nodiscard]] uint32_t neighbour(std::size_t index) const;

  private:

    uint32_t m_ip;
};

#endif // COMMS_H_
