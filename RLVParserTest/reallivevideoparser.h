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
    void importFinished(QList<RealLiveVideo> rlvs);

private slots:
    void rlvFilesFound();
    void importReady();

private:
};

#endif // REALLIVEVIDEOPARSER_H
