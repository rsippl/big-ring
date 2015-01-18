#ifndef REALLIVEVIDEOPARSER_H
#define REALLIVEVIDEOPARSER_H

#include <QFile>
#include <QFutureWatcher>
#include <QObject>
#include <QSharedPointer>

#include "reallifevideo.h"

/**
 * @brief Importer for Tacx RLV files.
 *
 * This importer will asynchronously import files and emit the signal importReady when the import is finished.
 * Call ::parseRealLiveVideoFilesFromDir with a root directory. When ready, ::importReady will be emitted.
 *
 * This importer will search for files with the extension .rlv and, for each of those files, find the corresponding
 * .pgmf and .avi file. The result will be a list of RealLifeVideo objects. Only slope-based files will be found,
 * power-based rlv files will be ommitted for now.
 */
class RealLifeVideoImporter: public QObject
{
    Q_OBJECT
public:
    explicit RealLifeVideoImporter(QObject* parent = NULL);

    /**
     * @brief parse the Real Life Videos from a root directory.
     * @param the root directory to search files from.
     */
    void parseRealLiveVideoFilesFromDir(const QString &root);

signals:
    /**
     * @brief signal emitted when the import is finished.
     * @param rlvs list of RealLifeVideo objects.
     */
    void importFinished(RealLifeVideoList rlvs);

private:
    void importReady(const RealLifeVideoList &rlvs);
};

#endif // REALLIVEVIDEOPARSER_H
