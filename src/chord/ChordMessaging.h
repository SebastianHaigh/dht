#ifndef CHORD_MESSAGING_H_
#define CHORD_MESSAGING_H_

#include "../comms/Comms.h"
#include "NodeId.h"

namespace chord {

class FindSuccessorMessage : public Message
{
  public:
    FindSuccessorMessage(CommsVersion version, const NodeId& nodeId);
    explicit FindSuccessorMessage(CommsVersion version);
    ~FindSuccessorMessage() = default;

    [[nodiscard]] EncodedMessage encode() const override;
    void decode(const EncodedMessage& message) override;

    [[nodiscard]] const NodeId& nodeId() const;

  private:
    NodeId m_nodeId;
};

}

#endif // CHORD_MESSAGING_H_

