#ifndef ANALYTICSSENDER_H
#define ANALYTICSSENDER_H

#include <QtCore/QObject>
#include <QtCore/QTimer>
#include <QtCore/QUuid>
#include <QtNetwork/QNetworkAccessManager>

#include "reallifevideo.h"
class AnalyticsSender : public QObject
{
    Q_OBJECT
public:
    explicit AnalyticsSender(QObject *parent = 0);

signals:

public slots:
private slots:
    void sendAnalyticsUpdate();
    void replyReceived();
private:
    QNetworkAccessManager *_networkAccessManager;
    QTimer *_sendTimer;

    QUuid _uuid;
    RealLifeVideo _currentRlv;
};

#endif // ANALYTICSSENDER_H
