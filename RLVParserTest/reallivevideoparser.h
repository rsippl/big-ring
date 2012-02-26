#ifndef REALLIVEVIDEOPARSER_H
#define REALLIVEVIDEOPARSER_H

#include <QFile>

#include "reallivevideo.h"

class RealLiveVideoParser
{
public:
	RealLiveVideoParser();

	RealLiveVideo parseRealLiveVideoFile(QFile& rlvFile) const;
};

#endif // REALLIVEVIDEOPARSER_H
