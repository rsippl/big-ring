

#include <QApplication>
#include <QtDebug>

#include "mainwindow.h"

#include "reallivevideoimporter.h"

#include "antcontroller.h"


int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
	app.setApplicationName("Indoor Cycling");

	qDebug() << "starting up";
	QString filename(argv[1]);
	RealLiveVideoImporter parser;
	ANTController antController(&app);
	MainWindow mw(parser, antController);



	mw.show();

//	parser.parseRealLiveVideoFilesFromDir(filename);

	app.exec();
}
