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



#include <QtWidgets/QApplication>
#include <QtDebug>
#include <gst/gst.h>
#include "antcontroller.h"
#include "reallifevideoimporter.h"
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    gst_init(&argc, &argv);
	app.setApplicationName("Indoor Cycling");

	qDebug() << "starting up";
	QString filename(argv[1]);
	RealLifeVideoImporter parser;
	ANTController antController(&app);

	Cyclist cyclist;
	MainWindow mw(parser, cyclist, antController);

	app.connect(&antController, SIGNAL(heartRateMeasured(quint8)), &cyclist, SLOT(setHeartRate(quint8)));
	app.connect(&antController, SIGNAL(cadenceMeasured(quint8)), &cyclist, SLOT(setCadence(quint8)));
	app.connect(&antController, SIGNAL(powerMeasured(quint16)), &cyclist, SLOT(setPower(quint16)));

    app.installEventFilter(&mw);
	if (argc >= 3) {
		QString robot(argv[2]);
		if (robot == "robot") {
            cyclist.setHeartRate(145u);
            cyclist.setPower(280);
            cyclist.setCadence(95u);
		}
	}

	mw.show();
	app.connect(&app, SIGNAL(lastWindowClosed()), &app, SLOT(quit()));
    app.connect(&app, SIGNAL(aboutToQuit()), &antController, SLOT(quit()));
	parser.parseRealLiveVideoFilesFromDir(filename);

	app.exec();
}
