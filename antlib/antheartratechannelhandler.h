#ifndef ANTHEARTRATECHANNELHANDLER_H
#define ANTHEARTRATECHANNELHANDLER_H

#include <QObject>

#include "antchannelhandler.h"
namespace indoorcycling {
class AntHeartRateChannelHandler : public AntChannelHandler
{
    Q_OBJECT
public:
    explicit AntHeartRateChannelHandler(int channelNumber, QObject *parent = 0);
signals:

public slots:
protected:
    virtual void handleBroadCastMessage(const BroadCastMessage& message) override;
private:
    HeartRateMessage _lastMessage;
};
}
#endif // ANTHEARTRATECHANNELHANDLER_H
