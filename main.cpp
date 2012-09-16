#include <QtCore/QCoreApplication>

#include <QApplication>
#include <QMainWindow>
#include <cstddef>

#include "mainwindow.h"
#include <QByteArray>
#include <QFile>
#include <QFutureSynchronizer>
#include <QHBoxLayout>
#include <QList>
#include <QString>
#include <QtDebug>
#include <QDirIterator>
#include <QtConcurrentRun>
#include "reallivevideo.h"
#include "reallivevideoimporter.h"
#include "reallivevideowidget.h"
#include "rlvlistwidget.h"
#include "videowidget.h"

struct frameDistanceMapping {
	quint32 frameNumber;
	float metersPerFrame;

	QString toString() const {
		return QString("\tframeNumber %1, meters per frame %2").arg(frameNumber).arg(metersPerFrame);
	}
};


struct generalPgmf {
	quint32 checksum;
	quint8 _courseName[34];
	qint32 powerSlopeOrHr;
	qint32 _timeOrDistance;
	double _totalTimeOrDistance;
	double energyCons;
	float startAltitude;
	qint32 breakCategory;

	QString courseName() const {
		quint8 firstBytes[34/2];
		for(quint32 i = 0; i < sizeof(firstBytes); ++i)
			firstBytes[i] = _courseName[i*2];
		return QString((char*)firstBytes);
	}

	QString type() const {
		if (powerSlopeOrHr == 0)
			return QString("Power");
		if (powerSlopeOrHr == 1)
			return QString("Slope");
		else
			return QString("HR");
	}

	QString timeOrDistance() const {
		if (_timeOrDistance == 0)
			return "Time";
		else
			return "Distance";
	}

	QString totalTimeOrDistance() const {
		if (_timeOrDistance == 0)
			return QString("%1 seconds").arg(_totalTimeOrDistance);
		else
			return QString("%1 km").arg(_totalTimeOrDistance/1000.0);
	}

	QString toString() const {
		return QString("General pgmf: %1, type: %2, %3-based, start alt: %4")
				.arg(courseName())
				.arg(type())
				.arg(timeOrDistance())
				.arg(startAltitude);
	}
};


generalPgmf readGeneralPgmf(QFile &file) {
	struct generalPgmf generalBlock;
	file.read((char*) &generalBlock.checksum, sizeof(qint32));
	file.read((char*) &generalBlock._courseName, 34);
	file.read((char*) &generalBlock.powerSlopeOrHr, sizeof(generalBlock) - offsetof(struct generalPgmf, powerSlopeOrHr));

	qDebug() << generalBlock.toString();

	return generalBlock;
}

struct program {
	float durationDistance;
	float powerSlopeHeartRate;
	float rollingFriction;

	QString toString() const {
		return QString("duration or distance: %1, value: %2, friction: %3")
				.arg(durationDistance).arg(powerSlopeHeartRate)
				.arg(rollingFriction);
	}
};

QList<program> readProgram(QFile &file, qint32 count) {
	QList<program> programBlocks;
	programBlocks.reserve(count);

	for (qint32 i = 0; i < count; ++i) {
		struct program programBlock;
		file.read((char*) &programBlock, sizeof(programBlock));
		programBlocks << programBlock;
		qDebug() << programBlock.toString();
	}
	return programBlocks;
}

QList<frameDistanceMapping> readFrameDistanceMapping(QFile &rlvFile, qint32 count) {
	QList<frameDistanceMapping> mappings;
	mappings.reserve(count);

	for (qint32 i = 0; i < count; i++) {
		struct frameDistanceMapping frameDistanceMappingBlock;
		rlvFile.read((char*) &frameDistanceMappingBlock, sizeof(frameDistanceMappingBlock));
		mappings << frameDistanceMappingBlock;

		qDebug() << frameDistanceMappingBlock.toString();
	}
	return mappings;
}

void readRest(QFile &rlvFile, qint32 size) {
	rlvFile.read(size);
}

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
	app.setApplicationName("Indoor Cycling");

	qDebug() << "starting up";
	QString filename(argv[1]);
	RealLiveVideoImporter parser;
	MainWindow mw(parser);

	mw.show();

	parser.parseRealLiveVideoFilesFromDir(filename);

	app.exec();
}
