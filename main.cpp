

#include <QApplication>
#include <QtDebug>

#include "mainwindow.h"

#include "reallivevideoimporter.h"

#include "ANT.h"


int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
	app.setApplicationName("Indoor Cycling");

	qDebug() << "starting up";
	QString filename(argv[1]);
	RealLiveVideoImporter parser;
	MainWindow mw(parser);
	ANT ant(&app);
	mw.show();

	parser.parseRealLiveVideoFilesFromDir(filename);

	app.exec();
}
