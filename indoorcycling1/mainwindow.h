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

#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPushButton>
#include <QTime>
#include <QTimer>

#include "cyclist.h"
#include "reallifevideo.h"
#include "simulation.h"
#include "newvideowidget.h"
#include <QtWidgets/QLabel>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QBoxLayout>
#include <QtWidgets/QSystemTrayIcon>


class ANTController;
class RealLifeVideoImporter;
class RlvListWidget;

class VideoController;
class VideoWidget;

namespace indoorcycling {
class ScreenSaverBlocker;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit
    MainWindow(const RealLifeVideoImporter& parser, Cyclist& cyclist, ANTController& antController, QWidget *parent = 0);

    void restoreWindow();
    void gotoFullScreen();

    bool eventFilter(QObject *obj, QEvent *event);
signals:
    void importFinished(RealLifeVideoList rlvs);

protected:
    virtual void focusOutEvent(QFocusEvent*);
    virtual void focusInEvent(QFocusEvent*);

    virtual void closeEvent(QCloseEvent *);

private slots:
    void rlvSelected(RealLifeVideo rlv);

    void antDeviceFound(QString description);

private:

    QLayout* setUpMain(QWidget *centralWidget);
    QLayout* setupSideBar(QWidget *centralWidget);
    QLabel* createLabel(const QString& text, QColor color, QWidget* centralWidget);

    void removeMargins();
    void restoreMargins();

    ANTController& _antController;
    RealLifeVideo _rlv;
    Cyclist& _cyclist;
    Simulation _simulation;
//    NewVideoWidget* videoWidget;
    QListWidget* courseListWidget;
    RlvListWidget* rlvListWidget;
    QMargins _margins;
    QRect _cachedGeometry;
    QBoxLayout* _layout;
    QPushButton* playButton;

    QSystemTrayIcon* _trayIcon;
    indoorcycling::ScreenSaverBlocker* _screenSaverBlocker;
};

#endif // MAINWINDOW_H
