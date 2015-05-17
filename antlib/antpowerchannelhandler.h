#ifndef ANTPOWERCHANNELHANDLER_H
#define ANTPOWERCHANNELHANDLER_H

#include "antchannelhandler.h"
namespace indoorcycling
{
class AntPowerChannelHandler : public AntChannelHandler
{
    Q_OBJECT
public:
    explicit AntPowerChannelHandler(int channelNumber, QObject* parent);
    virtual ~AntPowerChannelHandler() {}
protected:
    virtual void handleBroadCastMessage(const BroadCastMessage& message) override;
private:
    PowerMessage _lastPowerMessage;
};
}
#endif // ANTPOWERCHANNELHANDLER_H
