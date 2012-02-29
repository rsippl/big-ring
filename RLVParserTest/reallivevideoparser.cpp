#include "reallivevideoparser.h"

#include <QtDebug>

RealLiveVideoParser::RealLiveVideoParser()
{
	// noop
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
	qDebug() << infoBlock.toString();

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

RealLiveVideo RealLiveVideoParser::parseRealLiveVideoFile(QFile &rlvFile) const
{
	rlvFile.open(QIODevice::ReadOnly);

	VideoInformation videoInformation(QString("Unknown"), 0.0);

	header_t header = readHeader(rlvFile);
	for(qint32 blockNr = 0; blockNr < header.numberOfBlocks; ++blockNr) {
			info_t infoBlock = readInfo(rlvFile);
			if (infoBlock.fingerprint == 2010) {
				generalRlv_t generalRlv = readGeneralRlv(rlvFile);
				videoInformation = VideoInformation(generalRlv.filename(), generalRlv.frameRate);
			}
	}
	return RealLiveVideo(videoInformation);
}
