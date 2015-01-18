/*
 * Copyright (c) 2012-2015 Ilja Booij (ibooij@gmail.com)
 *
 * This file is part of Big Ring Indoor Video Cycling
 *
 * Big Ring Indoor Video Cycling is free software: you can redistribute 
 * it and/or modify it under the terms of the GNU General Public License 
 * as published by the Free Software Foundation, either version 3 of the 
 * License, or (at your option) any later version.
 *
 * Big Ring Indoor Video Cycling  is distributed in the hope that it will 
 * be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with Big Ring Indoor Video Cycling.  If not, see 
 * <http://www.gnu.org/licenses/>.
 */

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
