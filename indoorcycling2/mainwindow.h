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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtCore/QScopedPointer>
#include <QtWidgets/QWidget>
#include <QtWidgets/QStackedWidget>

#include "antcontroller.h"
#include "reallifevideo.h"
#include "reallifevideoimporter.h"

class Cyclist;
class VideoListView;
class NewVideoWidget;
class Run;
class Simulation;

namespace Ui {
class MainWindow;
}

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    explicit MainWindow(QString dir, QWidget *parent = 0);
    ~MainWindow();

protected:
    virtual void keyPressEvent(QKeyEvent *);
    virtual void closeEvent(QCloseEvent *);
private slots:
    void importFinished(RealLifeVideoList rlvs);
private:
    void step();
    void startRun(RealLifeVideo rlv, int courseNr);

    RealLifeVideoImporter *_importer;

    ANTController* _antController;
    Cyclist* _cyclist;
    Simulation* _simulation;
    QScopedPointer<Run,QScopedPointerDeleteLater> _run;

    QStackedWidget* const _stackedWidget;
    VideoListView* const _listView;
    NewVideoWidget* const _videoWidget;
    QRect _savedGeometry;
};

#endif // MAINWINDOW_H
