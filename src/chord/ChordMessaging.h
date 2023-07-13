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
                                 uint32_t requestId);
    explicit FindSuccessorResponseMessage(CommsVersion version);
    ~FindSuccessorResponseMessage() = default;

    [[nodiscard]] EncodedMessage encode() const override;
    void decode(const EncodedMessage& message) override;

    [[nodiscard]] const NodeId& nodeId() const;
    [[nodiscard]] const NodeId& sourceNodeId() const;
    [[nodiscard]] uint32_t requestId() const;

  private:
    NodeId m_nodeId;
    NodeId m_sourceNodeId;
    uint32_t m_requestId;
};

}

#endif // CHORD_MESSAGING_H_

