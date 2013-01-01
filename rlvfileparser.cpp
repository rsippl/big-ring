#include "rlvfileparser.h"

#include <QDir>
#include <QFileInfo>
#include <QtDebug>

RlvFileParser::RlvFileParser(const QStringList& videoFilenames): TacxFileParser(),
	_videoFilenames(videoFilenames)
{
}

QString RlvFileParser::findVideoFilename(const QStringList& videoFilenames, const QString& rlvVideoFilename)
{
	QString normalizedVideoFilename;
	// if rlv video filename contains a path, get only the last part of it.
	normalizedVideoFilename = rlvVideoFilename.split("\\").last();

	foreach(const QString& videoFilename, videoFilenames) {
		QFileInfo fileInfo(videoFilename);
		if (fileInfo.fileName().toLower() == normalizedVideoFilename.toLower())
			return videoFilename;
	}

	return QString();
}

QString RlvFileParser::findPgmfFilename(QFile &rlvFile)
{
	QFileInfo fileInfo(rlvFile);

	QString pgmfFilename(QString("%1.pgmf").arg(fileInfo.baseName()));
	QFileInfo pgmfFileInfo(fileInfo.dir(), pgmfFilename);

	return pgmfFileInfo.filePath();
}

RealLiveVideo RlvFileParser::parseRlvFile(QFile &rlvFile)
{
	if (!rlvFile.open(QIODevice::ReadOnly))
		return RealLiveVideo();

	QFile pgmfFile(findPgmfFilename(rlvFile));
	if (!pgmfFile.open(QIODevice::ReadOnly))
		return RealLiveVideo();

	Profile profile = PgmfFileParser().readProfile(pgmfFile);

	QList<Course> courses;
	QString name = QFileInfo(rlvFile).baseName();
	VideoInformation videoInformation(QString("Unknown"), 0.0);
	QList<DistanceMappingEntry> distanceMapping;

	tacxfile::header_t header = readHeaderBlock(rlvFile);
	for(qint32 blockNr = 0; blockNr < header.numberOfBlocks; ++blockNr) {
		tacxfile::info_t infoBlock = readInfoBlock(rlvFile);
		if (infoBlock.fingerprint < 0 || infoBlock.fingerprint > 10000)
			break;
		if (infoBlock.fingerprint == 2010) {
			tacxfile::generalRlv_t generalRlv = readGeneralRlvBlock(rlvFile);
			QString videoFilename = findVideoFilename(_videoFilenames, generalRlv.filename());
			videoInformation = VideoInformation(videoFilename, generalRlv.frameRate);
		}
		else if (infoBlock.fingerprint == 2020) {
			distanceMapping = readFrameDistanceMapping(rlvFile, infoBlock.numberOfRecords);
		}
		else if (infoBlock.fingerprint == 2040) {
			courses = readCourseInformation(rlvFile, infoBlock.numberOfRecords);
		}
		else {
			rlvFile.read(infoBlock.numberOfRecords * infoBlock.recordSize);
		}
	}
	return RealLiveVideo(name, videoInformation, courses, distanceMapping, profile);
}

tacxfile::header_t TacxFileParser::readHeaderBlock(QFile &rlvFile)
{
	tacxfile::header_t headerBlock;
	rlvFile.read((char*) &headerBlock, sizeof(headerBlock));

	return headerBlock;
}

tacxfile::info_t TacxFileParser::readInfoBlock(QFile &rlvFile)
{
	tacxfile::info_t infoBlock;
	rlvFile.read((char*) &infoBlock, sizeof(infoBlock));
	return infoBlock;
}

tacxfile::generalRlv_t RlvFileParser::readGeneralRlvBlock(QFile &rlvFile)
{
	tacxfile::generalRlv_t generalBlock;
	rlvFile.read((char*) &generalBlock._filename, 522);
	rlvFile.read((char*) &generalBlock.frameRate, sizeof(float));
	rlvFile.read((char*) &generalBlock.originalRunWeight, sizeof(float));
	rlvFile.read((char*) &generalBlock.frameOffset, sizeof(qint32));

	return generalBlock;
}

QList<Course> RlvFileParser::readCourseInformation(QFile &rlvFile, qint32 count)
{
	QList<Course> courses;
	for (qint32 i = 0; i < count; i++) {
		tacxfile::rlvCourseInformation_t courseInfoBlock;
		rlvFile.read((char*) &courseInfoBlock.start, sizeof(float));
		rlvFile.read((char*) &courseInfoBlock.end, sizeof(float));
		rlvFile.read((char*) &courseInfoBlock._courseName, 66);
		rlvFile.read((char*) &courseInfoBlock._textFilename, 522);

		courses.append(Course(courseInfoBlock.courseName(), courseInfoBlock.start, courseInfoBlock.end));
	}
	return courses;
}

QList<DistanceMappingEntry> RlvFileParser::readFrameDistanceMapping(QFile &rlvFile, qint32 count)
{
	QList<DistanceMappingEntry> mappings;
	mappings.reserve(count);

	for (qint32 i = 0; i < count; i++) {
		tacxfile::frameDistanceMapping_t frameDistanceMappingBlock;
		rlvFile.read((char*) &frameDistanceMappingBlock, sizeof(frameDistanceMappingBlock));
		mappings << DistanceMappingEntry(frameDistanceMappingBlock.frameNumber, frameDistanceMappingBlock.metersPerFrame);
	}
	return mappings;
}


Profile PgmfFileParser::readProfile(QFile &pgmfFile)
{
	tacxfile::header_t header = readHeaderBlock(pgmfFile);
	tacxfile::generalPgmf_t generalBlock;
	QList<tacxfile::program_t> profileBlocks;

	for(qint32 blockNr = 0; blockNr < header.numberOfBlocks; ++blockNr) {
		tacxfile::info_t infoBlock = readInfoBlock(pgmfFile);
		if (infoBlock.fingerprint < 0 || infoBlock.fingerprint > 10000)
			break;
		else if (infoBlock.fingerprint == 1010)
			generalBlock = readGeneralPgmfInfo(pgmfFile);
		else if (infoBlock.fingerprint == 1020)
			profileBlocks = readProgram(pgmfFile, infoBlock.numberOfRecords);
		else
			pgmfFile.read(infoBlock.numberOfRecords * infoBlock.recordSize);

	}

	QMap<float, ProfileEntry> profile;
	if (generalBlock.powerSlopeOrHr == 1) {
		float currentDistance = 0.0f;
		QListIterator<tacxfile::program_t> it(profileBlocks);
		while(it.hasNext()) {
			tacxfile::program_t item = it.next();
			profile[currentDistance] = ProfileEntry(item.durationDistance, item.powerSlopeHeartRate);
			currentDistance += item.durationDistance;
		}
	}
	ProfileType type = (ProfileType) generalBlock.powerSlopeOrHr;

	return Profile(type, generalBlock.startAltitude, profile);
}

tacxfile::generalPgmf_t PgmfFileParser::readGeneralPgmfInfo(QFile &pgmfFile)
{
	tacxfile::generalPgmf_t generalBlock;
	pgmfFile.read((char*) &generalBlock.checksum, sizeof(qint32));
	pgmfFile.read((char*) &generalBlock._courseName, 34);
	pgmfFile.read((char*) &generalBlock.powerSlopeOrHr, sizeof(generalBlock) - offsetof(tacxfile::generalPgmf_t, powerSlopeOrHr));

	return generalBlock;
}

QList<tacxfile::program_t> PgmfFileParser::readProgram(QFile &pgmfFile, quint32 count)
{
	QList<tacxfile::program_t> programBlocks;
	programBlocks.reserve(count);

	for (quint32 i = 0; i < count; ++i) {
		tacxfile::program_t programBlock;
		pgmfFile.read((char*) &programBlock, sizeof(programBlock));
		programBlocks << programBlock;
	}
	return programBlocks;
}
