

#include <QApplication>
#include <QtDebug>


#include "antcontroller.h"
#include "CommPort.h"
#include "reallivevideoimporter.h"
#include "mainwindow.h"
#include "Serial.h"


int main(int argc, char *argv[])
{
	CommPort::addListFunction(&Serial::myListCommPorts);
	QApplication app(argc, argv);
	app.setApplicationName("Indoor Cycling");

	qDebug() << "starting up";
	QString filename(argv[1]);
	RealLiveVideoImporter parser;
	ANTController antController(&app);
	MainWindow mw(parser, antController);



	mw.show();

	parser.parseRealLiveVideoFilesFromDir(filename);

	app.exec();
}
