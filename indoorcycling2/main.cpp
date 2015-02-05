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
#include <QApplication>
#include <gst/gst.h>

void loadStyleSheet(QApplication& a)
{
    QFile file(":/ui/QTDark.stylesheet");
    file.open(QIODevice::ReadOnly);
    QString styleSheet = QLatin1String(file.readAll());
    a.setStyleSheet(styleSheet);
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    loadStyleSheet(a);

    a.setOrganizationDomain("org.github.ibooij");
    a.setOrganizationName("Ilja Booij");
    a.setApplicationName("Big Ring Video Indoor Cycling");

    gst_init(&argc, &argv);
    QString dir;
    if (a.arguments().length() > 1) {
        dir = a.arguments()[1];
    } else {
        dir = "/media/ibooij/SamsungTacx/Tacx/";
    }
    MainWindow w(dir);
    w.showMaximized();

    return a.exec();

    gst_deinit();
}
