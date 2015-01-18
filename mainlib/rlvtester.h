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
