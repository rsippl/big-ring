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
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QWidget>
#include <QtWidgets/QStackedWidget>

#include "reallifevideo.h"
#include "reallifevideoimporter.h"
#include "util.h"

class Cyclist;
class VideoListView;
class NewVideoWidget;
class Run;
class Simulation;

namespace indoorcycling {
class AntCentralDispatch;
}
namespace Ui {
class MainWindow;
}

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    virtual void keyPressEvent(QKeyEvent *);
    virtual void closeEvent(QCloseEvent *);
    virtual void showEvent(QShowEvent *);
private slots:
    void loadVideos();
    void importFinished(RealLifeVideoList rlvs);
    void removeDisplayMessage();
private:
    void setupMenuBar();
    void step();
    void startRun(RealLifeVideo rlv, int courseNr);

    indoorcycling::AntCentralDispatch* const _antCentralDispatch;

    qobject_unique_ptr<Run> _run;

    QMenuBar* const _menuBar;
    QStackedWidget* const _stackedWidget;
    QAction* _showPreferencesAction;

    VideoListView* const _listView;
    QScopedPointer<NewVideoWidget> _videoWidget;
    QRect _savedGeometry;
};

#endif // MAINWINDOW_H
