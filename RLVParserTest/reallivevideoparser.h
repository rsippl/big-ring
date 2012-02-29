#ifndef REALLIVEVIDEOPARSER_H
#define REALLIVEVIDEOPARSER_H

#include <QFile>
#include <QSharedPointer>

#include "reallivevideo.h"

class RealLiveVideoParser
{
public:
	explicit RealLiveVideoParser();

	RealLiveVideo parseRealLiveVideoFile(QFile& rlvFile) const;
};

#endif // REALLIVEVIDEOPARSER_H
