#include "analyticssender.h"

#include <QtNetwork/QHostInfo>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>
#include <QtCore/QUrlQuery>

namespace {
const int SEND_DELAY_SECONDS = 5;
const QString URL = "https://www.google-analytics.com/collect";
}
AnalyticsSender::AnalyticsSender(QObject *parent) :
    QObject(parent), _networkAccessManager(new QNetworkAccessManager(this)), _sendTimer(new QTimer(this))
{
    _sendTimer->setInterval(SEND_DELAY_SECONDS * 1000);
    connect(_sendTimer, &QTimer::timeout, this, &AnalyticsSender::sendAnalyticsUpdate);
    _sendTimer->start();
    _uuid = QUuid::createUuid();
}

void AnalyticsSender::sendAnalyticsUpdate()
{
    QString hostname = QHostInfo::localHostName() + "." + QHostInfo::localDomainName();
    QNetworkRequest request(URL);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    QUrlQuery params;
    params.addQueryItem("v", "1");
    params.addQueryItem("tid", "UA-65087849-3");
    params.addQueryItem("cid", _uuid.toString());
    params.addQueryItem("t", "event");
    params.addQueryItem("ec", "action"); // Event category
    params.addQueryItem("ea", "browsing"); // Event action
    params.addQueryItem("el", "empty");// Event label
    params.addQueryItem("ev", "empty"); // Event value
//    params.addQueryItem("dh", hostname);
    params.addQueryItem("dp", "Main");
//    params.addQueryItem("dt", "Main");

    QNetworkReply *reply = _networkAccessManager->post(request, params.toString(QUrl::FullyEncoded).toUtf8());
    connect(reply, &QNetworkReply::finished, this, &AnalyticsSender::replyReceived);
}

void AnalyticsSender::replyReceived()
{
    QNetworkReply *reply = static_cast<QNetworkReply*>(sender());
    if (reply->error() == QNetworkReply::NoError) {
        int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        qDebug() << "no error. status code" << statusCode;
    } else {
        QString replyBody = QString::fromUtf8(reply->readAll().data());
        qDebug() << replyBody;
    }
    reply->deleteLater();
}
