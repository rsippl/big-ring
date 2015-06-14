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

#include "mainwindow.h"
#include <QtCore/QtGlobal>
#include <QtCore/QTime>
#include <QApplication>
#include <QtGui/QPalette>
#include <QtWidgets/QStyleFactory>
#include <cstdio>

namespace
{
FILE *logFile;
}
void prettyLogging(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QByteArray localMsg = msg.toLocal8Bit();
    QString now = QTime::currentTime().toString();
    const char* level;
    switch (type) {
    case QtDebugMsg:
        level = "DEBUG";
        break;
    case QtWarningMsg:
        level = "WARNING";
        break;
    case QtCriticalMsg:
        level = "CRITICAL";
        break;
    case QtFatalMsg:
        level = "FATAL";
    }
    fprintf(stderr, "[%s] %s [%s:%u] %s\n", qPrintable(now), level,
            context.function, context.line, localMsg.constData());
    fprintf(logFile, "[%s] %s [%s:%u] %s\n", qPrintable(now), level,
            context.function, context.line, localMsg.constData());
    if (type == QtFatalMsg) {
        abort();
    }
}

void loadStyleSheet(QApplication& a)
{
    a.setStyle(QStyleFactory::create("Fusion"));

    QPalette palette;
    palette.setColor(QPalette::Window, QColor(53,53,53));
    palette.setColor(QPalette::WindowText, Qt::white);
    palette.setColor(QPalette::Base, QColor(15,15,15));
    palette.setColor(QPalette::AlternateBase, QColor(53,53,53));
    palette.setColor(QPalette::ToolTipBase, QColor(Qt::green).lighter());
    palette.setColor(QPalette::ToolTipText, Qt::black);
    palette.setColor(QPalette::Text, Qt::white);;
    palette.setColor(QPalette::Button, QColor(53,53,53));
    palette.setColor(QPalette::ButtonText, Qt::white);
    palette.setColor(QPalette::BrightText, Qt::red);

    palette.setColor(QPalette::Highlight, QColor(Qt::green).lighter());
    palette.setColor(QPalette::HighlightedText, Qt::black);
    palette.setColor(QPalette::Disabled, QPalette::Text, Qt::darkGray);
    palette.setColor(QPalette::Disabled, QPalette::ButtonText, Qt::darkGray);

    a.setPalette(palette);
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    logFile = fopen("message.log", "a");

    qInstallMessageHandler(prettyLogging);
    loadStyleSheet(a);

    a.setOrganizationDomain("org.github.ibooij");
    a.setOrganizationName("Ilja Booij");
    a.setApplicationName("Big Ring Indoor Video Cycling");

    MainWindow w;
    w.showMaximized();

    return a.exec();
    fclose(logFile);
}
