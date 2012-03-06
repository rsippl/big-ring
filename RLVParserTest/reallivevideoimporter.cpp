#include "reallivevideoimporter.h"

#include <functional>
#include <QDirIterator>
#include <QFutureSynchronizer>
#include <QMap>
#include <QStringList>
#include <QtConcurrentMap>
#include <QtConcurrentRun>
#include <QtDebug>

RealLiveVideo parseRealLiveVideoFile(QFile &rlvFile, const QStringList& aviFiles);

RealLiveVideoImporter::RealLiveVideoImporter(QObject* parent): QObject(parent)
{
}

struct ParseRlvFunctor: public std::unary_function<const QString&,RealLiveVideo> {
    ParseRlvFunctor(const QStringList& aviFiles): _aviFiles(aviFiles) {}

    RealLiveVideo operator()(const QString& filename) const {
        QFile file(filename);
        return parseRealLiveVideoFile(file, _aviFiles);
    }

    QStringList _aviFiles;
};

QStringList findFiles(const QString& root, const QString& pattern)
{
    QStringList filters;
    filters << pattern;
    QDirIterator it(root, filters, QDir::NoFilter, QDirIterator::Subdirectories);

    QStringList filePaths;
    while(it.hasNext())
        filePaths << it.next();
    return filePaths;
}

QStringList findRlvFiles(QString& root)
{
    return findFiles(root, "*.rlv");
}

RealLiveVideoList importRlvFiles(QString root)
{
    QStringList filePaths = findRlvFiles(root);
    QStringList aviFiles = findFiles(root, "*.avi");

    QFuture<RealLiveVideo> rlvParserFuture = QtConcurrent::mapped(filePaths.begin(), filePaths.end(), ParseRlvFunctor(aviFiles));

    return rlvParserFuture.results();
}

void RealLiveVideoImporter::parseRealLiveVideoFilesFromDir(QString &root)
{
    QFutureWatcher<RealLiveVideoList> *futureWatcher = new QFutureWatcher<RealLiveVideoList>();
    connect(futureWatcher, SIGNAL(finished()), this, SLOT(importReady()));
    QFuture<QList<RealLiveVideo> > importRlvFuture = QtConcurrent::run(importRlvFiles, root);
    futureWatcher->setFuture(importRlvFuture);
}

void RealLiveVideoImporter::importReady()
{
    QFutureWatcher<RealLiveVideoList>* watcher = dynamic_cast<QFutureWatcher<RealLiveVideoList>*>(sender());
    if (!watcher)
        qWarning() << "Error getting result in " << Q_FUNC_INFO;
    RealLiveVideoList rlvList = watcher->future().result();

    // sort rlv list by name
    qSort(rlvList.begin(), rlvList.end(), RealLiveVideo::compareByName);
    emit importFinished(rlvList);
    watcher->deleteLater();
}

typedef struct header {
    qint16 fingerprint;
    qint16 version;
    qint32 numberOfBlocks;

    QString toString() const {
        return QString("fingerprint %1, version %2, number of blocks %3")
                .arg(fingerprint).arg(version).arg(numberOfBlocks);
    }
} header_t;

static header_t readHeader(QFile& rlvFile) {
    header_t headerBlock;
    rlvFile.read((char*) &headerBlock, sizeof(headerBlock));

    //	qDebug() << headerBlock.toString();

    return headerBlock;
}

typedef struct info {
    qint16 fingerprint;
    qint16 version;
    qint32 numberOfRecords;
    qint32 recordSize;

    qint32 size() const {
        return numberOfRecords * recordSize;
    }

    QString toString() const {
        return QString("block fingerprint %1, block version %2, number of records %3, record size %4")
                .arg(fingerprint).arg(version).arg(numberOfRecords).arg(recordSize);
    }
} info_t;

static info_t readInfo(QFile &rlvFile) {
    info_t infoBlock;

    rlvFile.read((char*) &infoBlock, sizeof(infoBlock));

    return infoBlock;
}

typedef struct generalRlv {
    quint8 _filename[522];
    float frameRate;
    float originalRunWeight;
    qint32 frameOffset;

    QString filename() const {
        quint8 firstBytes[522/2];
        for(quint32 i = 0; i < sizeof(firstBytes); ++i)
            firstBytes[i] = _filename[i*2];
        return QString((char*)firstBytes);

    }
    QString toString() const {
        return QString("video file name: %1, frame rate %2").arg(filename()).arg(frameRate);
    }
} generalRlv_t;

static generalRlv_t readGeneralRlv(QFile &rlvFile) {
    generalRlv_t generalBlock;
    rlvFile.read((char*) &generalBlock._filename, 522);
    rlvFile.read((char*) &generalBlock.frameRate, sizeof(float));
    rlvFile.read((char*) &generalBlock.originalRunWeight, sizeof(float));
    rlvFile.read((char*) &generalBlock.frameOffset, sizeof(qint32));
    qDebug() << generalBlock.toString();

    return generalBlock;
}

QString findVideoFilename(const QStringList& videoFilenames, const QString& rlvVideoFilename)
{
    foreach(QString videoFilename, videoFilenames) {
        QFileInfo fileInfo(videoFilename);
        if (fileInfo.fileName().toLower() == rlvVideoFilename.toLower())
            return videoFilename;
    }
    return QString();
}

RealLiveVideo parseRealLiveVideoFile(QFile &rlvFile, const QStringList& videoFilenames)
{
    if (!rlvFile.open(QIODevice::ReadOnly))
        return RealLiveVideo();

    QString name = QFileInfo(rlvFile).baseName();
    VideoInformation videoInformation(QString("Unknown"), 0.0);

    header_t header = readHeader(rlvFile);
    for(qint32 blockNr = 0; blockNr < header.numberOfBlocks; ++blockNr) {
	        info_t infoBlock = readInfo(rlvFile);
        if (infoBlock.fingerprint < 0 || infoBlock.fingerprint > 10000)
	break;
        if (infoBlock.fingerprint == 2010) {
            generalRlv_t generalRlv = readGeneralRlv(rlvFile);
            QString videoFilename = findVideoFilename(videoFilenames, generalRlv.filename());
            qDebug() << generalRlv.filename() << " => " << videoFilename;
            videoInformation = VideoInformation(videoFilename, generalRlv.frameRate);
        }
    }
    return RealLiveVideo(name, videoInformation);
}
