

#include <QtWidgets/QApplication>
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
	app.connect(&antController, SIGNAL(cadenceMeasured(quint8)), &cyclist, SLOT(setCadence(quint8)));
	app.connect(&antController, SIGNAL(powerMeasured(quint16)), &cyclist, SLOT(setPower(quint16)));

	if (argc >= 3) {
		QString robot(argv[2]);
		if (robot == "robot") {
			cyclist.setPower(600);
		}
	}

	mw.show();
	app.connect(&app, SIGNAL(lastWindowClosed()), &app, SLOT(quit()));
	parser.parseRealLiveVideoFilesFromDir(filename);

	app.exec();
}
