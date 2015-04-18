#ifndef ANTMESSAGE2TEST_H
#define ANTMESSAGE2TEST_H

#include <QObject>

class AntMessage2Test : public QObject
{
    Q_OBJECT
private slots:
    void systemReset();
    void setNetworkKey();
    void unassignChannel();
};

#endif // ANTMESSAGE2TEST_H
