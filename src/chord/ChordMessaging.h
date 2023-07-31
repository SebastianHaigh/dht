#ifndef CHORD_MESSAGING_H_
#define CHORD_MESSAGING_H_

#include "../comms/Comms.h"
#include "NodeId.h"
#include <cstdint>

namespace chord {

class FindSuccessorMessage : public Message
{
  public:
    FindSuccessorMessage(CommsVersion version,
                         const NodeId& nodeId,
                         const NodeId& sourceNodeId,
                         uint32_t requestId);
    explicit FindSuccessorMessage(CommsVersion version);
    ~FindSuccessorMessage() = default;

    [[nodiscard]] EncodedMessage encode() const override;
    void decode(const EncodedMessage& message) override;

    [[nodiscard]] const NodeId& queryNodeId() const;
    [[nodiscard]] const NodeId& sourceNodeId() const;
    [[nodiscard]] uint32_t requestId() const;

  private:
    NodeId m_nodeIdForQuery;
    NodeId m_sourceNodeId;
    uint32_t m_requestId;
};

class FindSuccessorResponseMessage : public Message
{
  public:
    FindSuccessorResponseMessage(CommsVersion version,
                                 const NodeId& nodeId,
                                 const NodeId& sourceNodeId,
                                 uint32_t ipAddress,
                                 uint32_t requestId);
    explicit FindSuccessorResponseMessage(CommsVersion version);
    ~FindSuccessorResponseMessage() = default;

    [[nodiscard]] EncodedMessage encode() const override;
    void decode(const EncodedMessage& message) override;

    [[nodiscard]] const NodeId& nodeId() const;
    [[nodiscard]] const NodeId& sourceNodeId() const;
    [[nodiscard]] uint32_t ip() const;
    [[nodiscard]] uint32_t requestId() const;

  private:
    NodeId m_nodeId;
    NodeId m_sourceNodeId;
    uint32_t m_ipAddress;
    uint32_t m_requestId;
};

class NotifyMessage : public Message
{
  public:
    NotifyMessage(CommsVersion version,
                  const NodeId& nodeId);
    explicit NotifyMessage(CommsVersion version);
    ~NotifyMessage() = default;

    [[nodiscard]] EncodedMessage encode() const override;
    void decode(const EncodedMessage& message) override;

    [[nodiscard]] const NodeId& nodeId() const;

  private:
    NodeId m_nodeId;
};

class GetNeighboursMessage : public Message
{
  public:
    GetNeighboursMessage(CommsVersion version,
                         const NodeId& sourceNodeId,
                         uint32_t requestId);
    explicit GetNeighboursMessage(CommsVersion version);
    ~GetNeighboursMessage() = default;

    [[nodiscard]] EncodedMessage encode() const override;
    void decode(const EncodedMessage& message) override;

    [[nodiscard]] const NodeId& sourceNodeId() const;
    [[nodiscard]] uint32_t requestId() const;

  private:
    NodeId m_sourceNodeId;
    uint32_t m_requestId;
};

class GetNeighboursResponseMessage : public Message
{
  public:
    GetNeighboursResponseMessage(CommsVersion version,
                                 const NodeId& successor,
                                 const NodeId& predecessor,
                                 const NodeId& sourceNodeId,
                                 uint32_t requestId);

    GetNeighboursResponseMessage(CommsVersion version,
                                 const NodeId& successor,
                                 const NodeId& sourceNodeId,
                                 uint32_t requestId);

    explicit GetNeighboursResponseMessage(CommsVersion version);
    ~GetNeighboursResponseMessage() = default;

    [[nodiscard]] EncodedMessage encode() const override;
    void decode(const EncodedMessage& message) override;

    [[nodiscard]] const NodeId& successor() const;
    [[nodiscard]] const NodeId& predecessor() const;
    [[nodiscard]] const NodeId& sourceNodeId() const;
    [[nodiscard]] bool hasPredecessor() const;
    [[nodiscard]] uint32_t requestId() const;

  private:
    NodeId m_successor;
    NodeId m_predecessor;
    NodeId m_sourceNodeId;
    bool m_hasPredecessor;
    uint32_t m_requestId;
};

}

#endif // CHORD_MESSAGING_H_

