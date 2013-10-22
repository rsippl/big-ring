#include "reallifevideoimporter.h"
#include "rlvfileparser.h"

#include <functional>
#include <QDirIterator>
#include <QFutureSynchronizer>
#include <QMap>
#include <QStringList>
#include <QtConcurrentMap>
#include <QtConcurrentRun>
#include <QtDebug>

RealLifeVideo parseRealLiveVideoFile(QFile &rlvFile, const QStringList& aviFiles);

RealLifeVideoImporter::RealLifeVideoImporter(QObject* parent): QObject(parent)
{
}

struct ParseRlvFunctor: public std::unary_function<const QString&,RealLifeVideo> {
	ParseRlvFunctor(const QStringList& aviFiles): _aviFiles(aviFiles) {}

	RealLifeVideo operator()(const QString& filename) const {
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

RealLifeVideoList importRlvFiles(QString root)
{
	QStringList filePaths = findRlvFiles(root);
	QStringList aviFiles = findFiles(root, "*.avi");

	QFuture<RealLifeVideo> rlvParserFuture = QtConcurrent::mapped(filePaths.begin(), filePaths.end(), ParseRlvFunctor(aviFiles));

	return rlvParserFuture.results();
}

void RealLifeVideoImporter::parseRealLiveVideoFilesFromDir(QString &root)
{
	QFutureWatcher<RealLifeVideoList> *futureWatcher = new QFutureWatcher<RealLifeVideoList>();
	connect(futureWatcher, SIGNAL(finished()), this, SLOT(importReady()));
	QFuture<QList<RealLifeVideo> > importRlvFuture = QtConcurrent::run(importRlvFiles, root);
	futureWatcher->setFuture(importRlvFuture);
}

void RealLifeVideoImporter::importReady()
{
	QFutureWatcher<RealLifeVideoList>* watcher = dynamic_cast<QFutureWatcher<RealLifeVideoList>*>(sender());
	if (!watcher)
		qWarning() << "Error getting result in " << Q_FUNC_INFO;
	RealLifeVideoList rlvList = watcher->future().result();

	// remove ergovideos and invalid rlvs
	QMutableListIterator<RealLifeVideo> it(rlvList);
	while(it.hasNext()) {
		const RealLifeVideo& rlv = it.next();
		if (!(rlv.isValid() && rlv.type() == SLOPE))
			it.remove();
	}

	// sort rlv list by name
	qSort(rlvList.begin(), rlvList.end(), RealLifeVideo::compareByName);

	emit importFinished(rlvList);
	watcher->deleteLater();
}




RealLifeVideo parseRealLiveVideoFile(QFile &rlvFile, const QStringList& videoFilenames)
{
	RlvFileParser parser(videoFilenames);
	return parser.parseRlvFile(rlvFile);
}
