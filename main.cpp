

#include <QApplication>
#include <QtDebug>
#include "antcontroller.h"
#include "reallivevideoimporter.h"
#include "mainwindow.h"

int main(int argc, char *argv[])
{
	qDebug() << "starting up";
	QApplication app(argc, argv);
	app.setApplicationName("Indoor Cycling");

	qDebug() << "starting up";
	QString filename(argv[1]);
	RealLiveVideoImporter parser;
	ANTController antController(&app);

	Cyclist cyclist;
	MainWindow mw(parser, cyclist, antController);

	app.connect(&antController, SIGNAL(heartRateMeasured(quint8)), &cyclist, SLOT(setHeartRate(quint8)));
	app.connect(&antController, SIGNAL(cadenceMeasured(float)), &cyclist, SLOT(setCadence(float)));
	app.connect(&antController, SIGNAL(powerMeasured(float)), &cyclist, SLOT(setPower(float)));

	if (argc >= 3) {
		QString robot(argv[2]);
		if (robot == "robot") {
			cyclist.setPower(300);
		}
	}

	mw.show();

	parser.parseRealLiveVideoFilesFromDir(filename);

	app.exec();
}
