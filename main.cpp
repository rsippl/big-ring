#include <QtCore/QCoreApplication>

#include <QApplication>
#include <QMainWindow>
#include <cstddef>

#include "mainwindow.h"
#include <QByteArray>
#include <QFile>
#include <QFutureSynchronizer>
#include <QHBoxLayout>
#include <QList>
#include <QString>
#include <QtDebug>
#include <QDirIterator>
#include <QtConcurrentRun>
#include "reallivevideo.h"
#include "reallivevideoimporter.h"
#include "reallivevideowidget.h"
#include "rlvlistwidget.h"
#include "videowidget.h"


int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
	app.setApplicationName("Indoor Cycling");

	qDebug() << "starting up";
	QString filename(argv[1]);
	RealLiveVideoImporter parser;
	MainWindow mw(parser);

	mw.show();

	parser.parseRealLiveVideoFilesFromDir(filename);

	app.exec();
}
