#include <QtGui/QGuiApplication>
#include <QtQml/QQmlContext>
#include <QtQuick/QQuickView>
#include <moviefiledroparea.h>
#include "game.h"
#include "antcontroller.h"
#include <QProcess>

int main(int argc, char *argv[])
{
	QGuiApplication app(argc, argv);
	app.setOverrideCursor(Qt::BlankCursor);
	QQuickView viewer;
	qmlRegisterType<MovieFileDropArea>("SufferfestPlayer", 0, 1, "MovieFileDropArea");
	Game game;
	ANTController antController;
	viewer.rootContext()->setContextProperty("game", &game);
	viewer.rootContext()->setContextProperty("ant", &antController);
	viewer.setSource(QUrl("qrc:/qml/SufferfestPlayer/main.qml"));
	viewer.setResizeMode(QQuickView::SizeRootObjectToView);

	QProcess p;
	quintptr winId = viewer.winId();
	QString command = QString("xdg-screensaver suspend %1").arg(winId);
	p.start(command);
	viewer.setHeight(600);
	viewer.setWidth(800);
	viewer.show();

	return app.exec();
}
