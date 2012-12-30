#include "reallivevideoimporter.h"
#include "rlvfileparser.h"

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

	// remove ergovideos and invalid rlvs
	QMutableListIterator<RealLiveVideo> it(rlvList);
	while(it.hasNext()) {
		const RealLiveVideo& rlv = it.next();
		if (!(rlv.isValid() && it.next().type() == SLOPE))
			it.remove();
	}

	// sort rlv list by name
	qSort(rlvList.begin(), rlvList.end(), RealLiveVideo::compareByName);

	emit importFinished(rlvList);
	watcher->deleteLater();
}




RealLiveVideo parseRealLiveVideoFile(QFile &rlvFile, const QStringList& videoFilenames)
{
	RlvFileParser parser(videoFilenames);
	return parser.parseRlvFile(rlvFile);
}
