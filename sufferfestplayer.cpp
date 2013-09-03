#include <QtGui/QGuiApplication>
#include <QtQml/QQmlContext>
#include <QtQuick/QQuickView>

#include "game.h"
#include "antcontroller.h"
#include <QProcess>

int main(int argc, char *argv[])
{
	QGuiApplication app(argc, argv);
	app.setOverrideCursor(Qt::BlankCursor);
	QQuickView viewer;

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
	viewer.showFullScreen();

	return app.exec();
}
