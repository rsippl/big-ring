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
#include <QtCore/QtDebug>
#include <QtCore/QTimer>
#include <QtCore/QUrl>

#include <QtWidgets/QAction>
#include <QtWidgets/QMenu>
#include <QtWidgets/QVBoxLayout>

#include "cyclist.h"
#include "run.h"
#include "settingsdialog.h"
#include "videolistview.h"
#include "newvideowidget.h"
#include "simulation.h"

MainWindow::MainWindow(QString dir, QWidget *parent) :
    QWidget(parent, Qt::Window),
    _importer(new RealLifeVideoImporter(this)),
    _antController(new ANTController(this)),
    _cyclist(new Cyclist(this)),
    _simulation(new Simulation(*_cyclist, this)),
    _menuBar(new QMenuBar),
    _stackedWidget(new QStackedWidget),
    _listView(new VideoListView)
{
    setupMenuBar();
    QVBoxLayout* layout = new QVBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);
    qDebug() << "starting from " << dir;
    connect(_importer, &RealLifeVideoImporter::importFinished, this, &MainWindow::importFinished);
    _importer->parseRealLiveVideoFilesFromDir(dir);

    _stackedWidget->addWidget(_listView);

    layout->addWidget(_menuBar);
    layout->addWidget(_stackedWidget);

    connect(_listView, &VideoListView::videoSelected, _listView, [=](RealLifeVideo& rlv, int courseNr) {
        startRun(rlv, courseNr);
    });
}

MainWindow::~MainWindow()
{
    // empty
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    qDebug() << "received key" << event->key() << event->text();
    switch(event->key()) {
    case Qt::Key_F:
        if (isFullScreen()) {
            showNormal();
            setGeometry(_savedGeometry);
        } else {
            _savedGeometry = geometry();
            showFullScreen();
        }
        break;
    case Qt::Key_M:
        if (isMaximized()) {
            showNormal();
        } else {
            showMaximized();
        }
        break;
    case Qt::Key_Escape:
        if (_run) {
            _run->stop();
        }
    default:
        QWidget::keyPressEvent(event);
        return;
        break;
    }
    event->accept();
}

void MainWindow::importFinished(RealLifeVideoList rlvs)
{
    _listView->setVideos(rlvs);
}

void MainWindow::setupMenuBar()
{
    QMenu* fileMenu = _menuBar->addMenu(tr("File"));

    QAction* showPreferencesAction = new QAction(tr("Preferences"), this);
    connect(showPreferencesAction, &QAction::triggered, showPreferencesAction, [=]() {
        SettingsDialog dialog(this);
        dialog.exec();
        update();
    });
    fileMenu->addAction(showPreferencesAction);

    QAction* quitAction = new QAction(tr("&Quit"), this);
    quitAction->setShortcut(QKeySequence::Quit);
    connect(quitAction, &QAction::triggered, this, &QWidget::close);
    fileMenu->addAction(quitAction);
    this->addAction(quitAction);


}

void MainWindow::startRun(RealLifeVideo rlv, int courseNr)
{
    Course course = rlv.courses()[courseNr];
    _videoWidget.reset(new NewVideoWidget(*_simulation));
    _stackedWidget->addWidget(_videoWidget.data());
    _run.reset(new Run(*_antController, _simulation, rlv, course, _videoWidget.data()));

    _stackedWidget->setCurrentIndex(_stackedWidget->indexOf(_videoWidget.data()));
    _savedGeometry = geometry();
    if (isMaximized()) {
        showFullScreen();
    }
    connect(_run.data(), &Run::stopped, _run.data(), [this]() {
        qDebug() << "run finished";
        _run.reset();
        _stackedWidget->setCurrentIndex(_stackedWidget->indexOf(_listView));
        _stackedWidget->removeWidget(_videoWidget.data());
        _videoWidget.reset();
        showNormal();
        setGeometry(_savedGeometry);
    });
    _run->start();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    qDebug() << "closing main window";
    if (_antController->isRunning()) {
        connect(_antController, &ANTController::finished, this, &QWidget::close);
        _antController->quit();
        event->ignore();
    } else {
        event->accept();
    }
}
