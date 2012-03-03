#ifndef REALLIVEVIDEOPARSER_H
#define REALLIVEVIDEOPARSER_H

#include <QFile>
#include <QFutureWatcher>
#include <QObject>
#include <QSharedPointer>

#include "reallivevideo.h"

class RealLiveVideoParser: public QObject
{
    Q_OBJECT
public:

    explicit RealLiveVideoParser(QObject* parent = NULL);

    void parseRealLiveVideoFilesFromDir(QString& root);

signals:
    void importFinished(RealLiveVideoList rlvs);

private slots:
    void importReady();

private:
};

#endif // REALLIVEVIDEOPARSER_H
