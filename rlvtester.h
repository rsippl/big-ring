#ifndef RLVTESTER_H
#define RLVTESTER_H

#include "reallifevideo.h"
#include "reallifevideoimporter.h"
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
	void importFinished(RealLifeVideoList rlvs);

private:
	bool testDistanceMapping(RealLifeVideo& rlv);

	RealLifeVideoImporter _importer;
};

#endif // RLVTESTER_H
