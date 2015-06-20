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
#ifdef Q_OS_LINUX
    qDebug() << "allowing screensaver to work again";
    QProcess* process = new QProcess;

    quintptr windowId = _window->winId();
    QString command = QString("xdg-screensaver resume %1").arg(windowId);
    connect(process, SIGNAL(finished(int)), process, SLOT(deleteLater()));
    process->start(command);
#endif
}

#ifdef Q_OS_LINUX
void ScreenSaverBlocker::blockScreenSaver()
{
    QProcess* process = new QProcess;

    quintptr windowId = _window->winId();
    QString command = QString("xdg-screensaver suspend %1").arg(windowId);
    connect(process, SIGNAL(error(QProcess::ProcessError)), this,
            SLOT(handleError(QProcess::ProcessError)));
    connect(process, SIGNAL(finished(int)), process, SLOT(deleteLater()));
    process->start(command);
}

#else
#ifdef Q_OS_WIN
void ScreenSaverBlocker::blockScreenSaver()
{
    qDebug() << "Screen saver blocking not implemented on windows yet.";
}
#endif
#endif

#ifdef Q_OS_LINUX
void ScreenSaverBlocker::handleError(QProcess::ProcessError error)
{
    QObject* process = sender();
    switch(error) {
    case QProcess::FailedToStart:
        qDebug() << "screensaver blocker could not be started.";
        break;
    default:
        qDebug() << "screensaver blocker error.";
    }
    process->deleteLater();
}
#endif
}
