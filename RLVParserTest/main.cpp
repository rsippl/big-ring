#include <QtCore/QCoreApplication>

#include <cstddef>

#include <QByteArray>
#include <QFile>
#include <QList>
#include <QString>
#include <QtDebug>

struct header {
	qint16 fingerprint;
	qint16 version;
	qint32 numberOfBlocks;

	QString toString() const {
		return QString("fingerprint %1, version %2, number of blocks %3")
				.arg(fingerprint).arg(version).arg(numberOfBlocks);
	}
};

struct info {
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
};

struct generalRlv {
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
};

struct frameDistanceMapping {
	quint32 frameNumber;
	float metersPerFrame;

	QString toString() const {
		return QString("\tframeNumber %1, meters per frame %2").arg(frameNumber).arg(metersPerFrame);
	}
};

header readHeader(QFile& rlvFile) {
	struct header headerBlock;
	rlvFile.read((char*) &headerBlock, sizeof(headerBlock));

//	qDebug() << headerBlock.toString();

	return headerBlock;
}

info readInfo(QFile &rlvFile) {
	struct info infoBlock;

	rlvFile.read((char*) &infoBlock, sizeof(infoBlock));
	qDebug() << infoBlock.toString();

	return infoBlock;
}

generalRlv readGeneralRlv(QFile &rlvFile) {
	struct generalRlv generalBlock;
	rlvFile.read((char*) &generalBlock._filename, 522);
	rlvFile.read((char*) &generalBlock.frameRate, sizeof(float));
	rlvFile.read((char*) &generalBlock.originalRunWeight, sizeof(float));
	rlvFile.read((char*) &generalBlock.frameOffset, sizeof(qint32));
	qDebug() << generalBlock.toString();

	return generalBlock;
}

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
    QCoreApplication a(argc, argv);

	QString filename(argv[1]);

	qDebug() << "Opening file: " << filename;
	QFile rlvFile(filename);
	if (!rlvFile.open(QIODevice::ReadOnly))
		return 1;

	struct header headerBlock = readHeader(rlvFile);
	for(qint32 blockNr = 0; blockNr < headerBlock.numberOfBlocks; ++blockNr) {
		struct info infoBlock = readInfo(rlvFile);
		if (infoBlock.fingerprint == 1010)
			readGeneralPgmf(rlvFile);
		else if (infoBlock.fingerprint == 1020)
			readProgram(rlvFile, infoBlock.numberOfRecords);
		else if (infoBlock.fingerprint == 2010)
			readGeneralRlv(rlvFile);
		else if (infoBlock.fingerprint == 2020)
			readFrameDistanceMapping(rlvFile, infoBlock.numberOfRecords);
		else if (infoBlock.fingerprint <= 6020) {
//			qDebug() << "block: " << infoBlock.fingerprint;
			readRest(rlvFile, infoBlock.size());
		} else {
			qDebug() << "Strange block, fingerprint is " << infoBlock.fingerprint;
			return 1;
		}
			
	}
	return 0;
}
