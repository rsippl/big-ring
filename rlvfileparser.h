#ifndef RLVFILEPARSER_H
#define RLVFILEPARSER_H

#include <QFile>
#include <QStringList>

#include "reallivevideo.h"

namespace tacxfile {



typedef struct header {
	qint16 fingerprint;
	qint16 version;
	qint32 numberOfBlocks;

	QString toString() const {
		return QString("fingerprint %1, version %2, number of blocks %3")
				.arg(fingerprint).arg(version).arg(numberOfBlocks);
	}
} header_t;

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

typedef struct rlvCourseInformation {
	float start;
	float end;
	quint8 _courseName[66];
	quint8 _textFilename[522];

	QString courseName() const {
		quint8 firstBytes[66/2];
		for(quint32 i = 0; i < sizeof(firstBytes); ++i)
			firstBytes[i] = _courseName[i*2];
		return QString((char*)firstBytes);
	}

	QString toString() const {
		return QString("start: %1, end %2, name: %3")
				.arg(start).arg(end).arg(courseName());
	}
} rlvCourseInformation_t;

typedef struct frameDistanceMapping {
	quint32 frameNumber;
	float metersPerFrame;

	QString toString() const {
		return QString("\tframeNumber %1, meters per frame %2").arg(frameNumber).arg(metersPerFrame);
	}
} frameDistanceMapping_t;
}

class RlvFileParser
{
public:
	RlvFileParser(const QStringList& videoFilenames);

	RealLiveVideo parseRlvFile(QFile& rlvFile);

	tacxfile::header_t readHeaderBlock(QFile& rlvFile);
	tacxfile::info_t readInfoBlock(QFile& rlvFile);
	tacxfile::generalRlv_t readGeneralRlvBlock(QFile& rlvFile);
	QList<Course> readCourseInformation(QFile& rlvFile, qint32 count);
	QList<DistanceMappingEntry> readFrameDistanceMapping(QFile& rlvFile, qint32 count);

private:
	QString findVideoFilename(const QStringList& videoFilenames, const QString& rlvVideoFilename);
	QString findPgmfFilename(QFile& rlvFile);
	const QStringList _videoFilenames;
};

#endif // RLVFILEPARSER_H
