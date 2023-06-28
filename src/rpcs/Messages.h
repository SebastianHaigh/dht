#include <cstddef>
#include <cstdint>
#include <vector>

using NodeId = int;

enum class MessageType : uint16_t
{
  JOIN,
  JOIN_RESPONSE,
  POSITION,
};

struct InternalMessage
{
  virtual ~InternalMessage() = default;

  virtual size_t encode(uint8_t* buffer, size_t bufferSize) const = 0;
  virtual void decode(uint8_t* buffer, size_t bufferSize) = 0;
};

// TODO (haigh) put this in its own file and test it
//   one that is done create a join message
struct PositionMessage : public InternalMessage
{
  PositionMessage();
  PositionMessage(NodeId id, const NodeId* neighbours, size_t numNeighbours);
  PositionMessage(NodeId id, std::vector<NodeId> neighbours);

  size_t encode(uint8_t* buffer, size_t bufferSize) const override;
  void decode(uint8_t* buffer, size_t bufferSize) override;

  static size_t messageSize() { return sizeof(MessageType) + sizeof(NodeId) * 6; };

  MessageType m_type = MessageType::POSITION;
  NodeId m_id;
  NodeId m_neighbours[5];
};

struct JoinMessage : public InternalMessage
{
  JoinMessage();
  explicit JoinMessage(NodeId id, uint32_t ip);
  
  size_t encode(uint8_t* buffer, size_t bufferSize) const override;
  void decode(uint8_t* buffer, size_t bufferSize) override;

  static size_t messageSize() { return sizeof(MessageType) + sizeof(NodeId) + sizeof(uint32_t); };

  MessageType m_type = MessageType::JOIN;
  NodeId m_id;
  uint32_t m_ip;
};

struct JoinResponseMessage : public InternalMessage
{
  JoinResponseMessage();
  explicit JoinResponseMessage(NodeId id);
  
  size_t encode(uint8_t* buffer, size_t bufferSize) const override;
  void decode(uint8_t* buffer, size_t bufferSize) override;

  static size_t messageSize() { return sizeof(MessageType) + sizeof(NodeId) + sizeof(uint32_t); };

  MessageType m_type = MessageType::JOIN_RESPONSE;
  NodeId m_id;
  uint32_t m_ip;
};
