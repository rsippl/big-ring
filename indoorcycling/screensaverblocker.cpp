#include "screensaverblocker.h"

#include <QtCore/QDebug>

#ifdef Q_OS_LINUX
#include <QtCore/QProcess>
#endif

namespace indoorcycling
{
ScreenSaverBlocker::ScreenSaverBlocker(QWidget* window, QObject *parent) :
    QObject(parent), _window(window)
{
    blockScreenSaver();
}

ScreenSaverBlocker::~ScreenSaverBlocker()
{
    qDebug() << "destroy";
}

#ifdef Q_OS_LINUX
void ScreenSaverBlocker::blockScreenSaver()
{
    connect(&_process, SIGNAL(error(QProcess::ProcessError)), this,
            SLOT(handleError(QProcess::ProcessError)));
    quintptr windowId = _window->winId();
    QString command = QString("xdg-screensaver suspend %1").arg(windowId);
    _process.start(command);
}

#else
#ifdef Q_OS_WINDOWS
void ScreenSaverBlocker::blockScreenSaver()
{
    qDebug() << "Screen saver blocking not implemented on windows yet."
}
#endif
#endif

void ScreenSaverBlocker::handleError(QProcess::ProcessError error)
{
    switch(error) {
    case QProcess::FailedToStart:
        qDebug() << "screensaver blocker could not be started.";
        break;
    default:
        qDebug() << "screensaver blocker error.";
    }
}
}
