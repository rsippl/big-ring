#ifndef ANTMESSAGETEST_H
#define ANTMESSAGETEST_H

#include <QObject>
#include "ANTMessage.h"

class AntMessageTest : public QObject
{
    Q_OBJECT
private slots:
    void systemReset();
    void setNetworkKey();

};

#endif // ANTMESSAGETEST_H
