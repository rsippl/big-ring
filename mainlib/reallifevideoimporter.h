#ifndef REALLIVEVIDEOPARSER_H
#define REALLIVEVIDEOPARSER_H

#include <QFile>
#include <QFutureWatcher>
#include <QObject>
#include <QSharedPointer>

#include "reallifevideo.h"

class RealLifeVideoImporter: public QObject
{
    Q_OBJECT
public:
    explicit RealLifeVideoImporter(QObject* parent = NULL);

    void parseRealLiveVideoFilesFromDir(const QString &root);

signals:
    void importFinished(RealLifeVideoList rlvs);

private slots:
    void importReady();

private:
};

#endif // REALLIVEVIDEOPARSER_H
