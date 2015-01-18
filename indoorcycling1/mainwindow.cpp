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

#include "antcontroller.h"
#include "reallifevideoimporter.h"
#include "rlvlistwidget.h"
#include "screensaverblocker.h"
#include "newvideowidget.h"
#include "run.h"
#include <QtWidgets/QApplication>
#include <QtGui/QKeyEvent>

#define  _GNU_SOURCE 1
#include <cmath>

MainWindow::MainWindow(const RealLifeVideoImporter& parser, Cyclist& cyclist, const ANTController& antController, QWidget *parent) :
    QMainWindow(parent), _antController(antController), _cyclist(cyclist), _simulation(_cyclist), _cachedGeometry(100, 100, 1024, 768),
    _screenSaverBlocker(new indoorcycling::ScreenSaverBlocker(this, this))
{
    setWindowIcon(QIcon("/home/ibooij/Downloads/alpehuez_mittel.jpg"));
    connect(&parser, SIGNAL(importFinished(RealLifeVideoList)), SIGNAL(importFinished(RealLifeVideoList)));

    setGeometry(_cachedGeometry);

    QWidget* centralWidget = new QWidget(this);
    _layout = new QHBoxLayout(centralWidget);
    _layout->addLayout(setupSideBar(centralWidget));

    _layout->addLayout(setUpMain(centralWidget));

    setCentralWidget(centralWidget);

    connect(rlvListWidget, &RlvListWidget::realLiveVideoSelected,
            &_simulation, &Simulation::rlvSelected);
//    connect(rlvListWidget, &RlvListWidget::realLiveVideoSelected, videoWidget, &NewVideoWidget::setRealLifeVideo);
//    connect(courseListWidget, &QListWidget::currentRowChanged, videoWidget, &NewVideoWidget::setCourseIndex);


    QObject::connect(this, SIGNAL(importFinished(RealLifeVideoList)), rlvListWidget, SLOT(setRealLiveVideos(RealLifeVideoList)));
    QObject::connect(rlvListWidget, SIGNAL(realLiveVideoSelected(RealLifeVideo)), SLOT(rlvSelected(RealLifeVideo)));
    QObject::connect(courseListWidget, SIGNAL(currentRowChanged(int)),
                     &_simulation, SLOT(courseSelected(int)));

    connect(playButton, SIGNAL(clicked(bool)), &_simulation, SLOT(play(bool)));

//    connect(&_cyclist, &Cyclist::distanceChanged, videoWidget, &NewVideoWidget::setDistance);

    connect(&antController, SIGNAL(deviceFound(QString)), SLOT(antDeviceFound(QString)));
    grabKeyboard();

    _trayIcon = new QSystemTrayIcon(this);
    _trayIcon->show();
}

QLayout* MainWindow::setUpMain(QWidget*)
{
    QVBoxLayout* layout = new QVBoxLayout();

//	videoWidget->setMinimumWidth(800);
//	videoWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

//	layout->addWidget(videoWidget);

    return layout;
}

QLayout* MainWindow::setupSideBar(QWidget* centralWidget)
{
    QVBoxLayout* layout = new QVBoxLayout();
    rlvListWidget = new RlvListWidget(centralWidget);
    rlvListWidget->setFixedWidth(300);
    layout->addWidget(rlvListWidget);

    courseListWidget = new QListWidget(centralWidget);
    courseListWidget->setFixedWidth(300);
    layout->addWidget(courseListWidget);

    playButton = new QPushButton("Play", centralWidget);

    connect(playButton, &QPushButton::clicked, playButton, [this]() {
        int row  = courseListWidget->currentIndex().row();
        Course course = _rlv.courses()[row];
        qFatal("Not implemented from here!");
        Run* run = nullptr;
        run->start();
        this->releaseKeyboard();
//        hide();
        connect(run, &Run::stopped, run, [this,run]() {
            qDebug() << "run finished";
            this->show();
            run->deleteLater();
        });
    });
//    playButton->setEnabled(videoWidget->isReadyToPlay());
//    connect(videoWidget, &NewVideoWidget::readyToPlay, playButton, &QPushButton::setEnabled);
    layout->addWidget(playButton);
    return layout;
}

void MainWindow::focusOutEvent(QFocusEvent *event)
{
    qDebug() << "MainWindow lost focus";
    QMainWindow::focusOutEvent(event);
}

void MainWindow::focusInEvent(QFocusEvent *event)
{
    qDebug() << "MainWindow gained focus";
    QMainWindow::focusInEvent(event);
}
QLabel *MainWindow::createLabel(const QString& text, QColor color, QWidget *centralWidget)
{
    QFont font;
    font.setPointSize(32);
    font.setBold(true);
    QPalette palette;
    palette.setColor(QPalette::Window, Qt::black);
    palette.setColor(QPalette::WindowText, color);

    QLabel* label = new QLabel(text, centralWidget);
    label->setAlignment(Qt::AlignCenter);
    label->setFont(font);
    label->setAutoFillBackground(true);
    label->setPalette(palette);

    return label;
}

void MainWindow::rlvSelected(RealLifeVideo rlv)
{
    courseListWidget->clear();
    foreach(const Course& course, rlv.courses()) {
        new QListWidgetItem(course.name(), courseListWidget);
    }
    if (courseListWidget->count() > 0)
        courseListWidget->setCurrentRow(0);
    _rlv = rlv;
}

void MainWindow::antDeviceFound(QString description)
{
    qDebug() << "ANT+ device found" << description;
    _trayIcon->showMessage("ANT+ device found", description);
}



void MainWindow::removeMargins()
{
    int l,t,r,b;

    _layout->getContentsMargins(&l,&t,&r,&b);
    _margins = QMargins(l,t,r,b);
    _layout->setContentsMargins(0, 0, 0, 0);
}

void MainWindow::restoreMargins()
{
    _layout->setContentsMargins(_margins);
}

void MainWindow::restoreWindow()
{
    if (isFullScreen()) {
        showNormal();
        setGeometry(_cachedGeometry);
        restoreMargins();
        rlvListWidget->show();
        courseListWidget->show();
        playButton->show();
        rlvListWidget->setEnabled(true);
        courseListWidget->setEnabled(true);
    }
}

void MainWindow::gotoFullScreen()
{
    rlvListWidget->setDisabled(true);
    courseListWidget->setDisabled(true);
    rlvListWidget->hide();
    courseListWidget->hide();
    playButton->hide();
    removeMargins();
    _cachedGeometry = geometry();
    showFullScreen();
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
           qDebug() << "key " << keyEvent->text() << "from" << obj;
    }
    return QObject::eventFilter(obj, event);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    event->accept();
}
