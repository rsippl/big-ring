#include "rlvtester.h"
#include <QFile>
#include <QtDebug>

RlvTester::RlvTester(QString, QObject *parent):
	QObject(parent)
{
	RlvFileParser parser;
	QFile rlvFile("/media/ibooij/SamsungTacx/Tacx/Batch1/Video/Amstel-Gold07.rlv");
	RealLifeVideo rlv = parser.parseRlvFile(rlvFile);
	testDistanceMapping(rlv);
//	connect(&_importer, SIGNAL(importFinished(RealLiveVideoList)), SLOT(importFinished(RealLiveVideoList)));
//	_importer.parseRealLiveVideoFilesFromDir(rootDirectory);
}

void RlvTester::importFinished(RealLifeVideoList rlvs)
{
	testDistanceMapping(rlvs[1]);
//	foreach(RealLiveVideo rlv, rlvs) {
//		qDebug() << "rlv: " << rlv.name();
//		testDistanceMapping(rlv);
//	}

	emit finished();
}

bool RlvTester::testDistanceMapping(RealLifeVideo &rlv)
{
	qDebug() << "testing for" << rlv.name() << rlv.totalDistance();
	quint32 lastFrame = 0;
	for(float currentDistance = 0; currentDistance < rlv.totalDistance(); currentDistance += .25f) {
		quint32 currentFrame = rlv.frameForDistance(currentDistance);

		if (lastFrame > currentFrame) {
			qDebug() << "error at distance" << currentDistance << "last frame was" << lastFrame << "current frame is" << currentFrame;
			return false;
		}
		lastFrame = currentFrame;
	}

	qDebug() << "finished testing (last frame:" << lastFrame << ")";
	return true;
}
