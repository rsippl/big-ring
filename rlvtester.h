#ifndef RLVTESTER_H
#define RLVTESTER_H

#include "reallivevideo.h"
#include "reallivevideoimporter.h"
#include "rlvfileparser.h"
#include <QObject>

class RlvTester : public QObject
{
	Q_OBJECT
public:
	explicit RlvTester(QString rootDirectory, QObject *parent = 0);

signals:
	void finished();
public slots:
	void importFinished(RealLiveVideoList rlvs);

private:
	bool testDistanceMapping(RealLiveVideo& rlv);

	RealLiveVideoImporter _importer;
};

#endif // RLVTESTER_H
