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

#include <functional>

#include <QtCore/QtDebug>
#include <QtCore/QTimer>
#include <QtCore/QUrl>

#include <QtWidgets/QAction>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QProgressDialog>
#include <QtWidgets/QVBoxLayout>

#include "ant/antcentraldispatch.h"
#include "model/cyclist.h"
#include "run.h"
#include "settingsdialog.h"
#include "videolistview.h"
#include "newvideowidget.h"
#include "model/simulation.h"

MainWindow::MainWindow(QWidget *parent) :
    QWidget(parent, Qt::Window),
    _antCentralDispatch(new indoorcycling::AntCentralDispatch(this)),
    _menuBar(new QMenuBar),
    _stackedWidget(new QStackedWidget),
    _listView(new VideoListView(this))
{
    _antCentralDispatch->initialize();
    setupMenuBar();
    QVBoxLayout* layout = new QVBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);

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
        if (_run) {
            if (isFullScreen()) {
                showNormal();
                setGeometry(_savedGeometry);
                _menuBar->show();
            } else {
                _savedGeometry = geometry();
                if (_run) {
                    _menuBar->hide();
                    showFullScreen();
                }
            }
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
            handleStopRun();
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

void MainWindow::removeDisplayMessage()
{
    if (_videoWidget) {
        _videoWidget->displayMessage("");
    }
}

/**
 * Asynchronously load all videos.
 */
void MainWindow::loadVideos()
{
    if (BigRingSettings().videoFolder().isEmpty()) {
        QMessageBox::StandardButton choice = QMessageBox::question(
                    this, tr("Video folder not configured"),
                    tr("No location was configured for the Real Life Videos.\n"
                       "Do you want to open Preferences now?"),
                    QMessageBox::StandardButtons(QMessageBox::Yes | QMessageBox::No),
                    QMessageBox::Yes);
        if (choice == QMessageBox::Yes) {
            _showPreferencesAction->trigger();
        }
        return;
    }

    RealLifeVideoImporter *importer = new RealLifeVideoImporter(this);

    QProgressDialog *progressDialog = new QProgressDialog("Importing Videos", QString(), 0, 0, this);

    connect(importer, &RealLifeVideoImporter::importFinished, this, [=](RealLifeVideoList list) {
        this->importFinished(list);
        importer->deleteLater();
        progressDialog->deleteLater();
    });
    connect(importer, &RealLifeVideoImporter::rlvFilesFound, importer, [progressDialog](int nr) {
        progressDialog->setMaximum(nr);
        progressDialog->setValue(0);
    });
    connect(importer, &RealLifeVideoImporter::rlvImported, importer, [progressDialog]() {
        qDebug() << "rlv imported!" << progressDialog->maximum() << progressDialog->value();
        int currentValue = progressDialog->value();
        progressDialog->setValue(currentValue + 1);
    });
    progressDialog->setValue(0);
    importer->importRealLiveVideoFilesFromDir();
    progressDialog->exec();
}

void MainWindow::setupMenuBar()
{
    QMenu* fileMenu = _menuBar->addMenu(tr("File"));

    _showPreferencesAction = new QAction(tr("Preferences"), this);
    connect(_showPreferencesAction, &QAction::triggered, _showPreferencesAction, [=]() {
        std::function<void(void)> videoLoadFunction(std::bind(&MainWindow::loadVideos, this));
        SettingsDialog dialog(_antCentralDispatch, videoLoadFunction, this);
        dialog.exec();
        update();
    });
    fileMenu->addAction(_showPreferencesAction);

    QAction* quitAction = new QAction(tr("&Quit"), this);
    quitAction->setShortcut(QKeySequence::Quit);
    connect(quitAction, &QAction::triggered, this, &QWidget::close);
    fileMenu->addAction(quitAction);
    this->addAction(quitAction);

    QMenu* helpMenu = _menuBar->addMenu(tr("Help"));
    QAction* aboutQtAction = new QAction(tr("About Qt"), this);
    connect(aboutQtAction, &QAction::triggered, this, [=]() {
        QMessageBox::aboutQt(this);
    });
    helpMenu->addAction(aboutQtAction);


}

void MainWindow::startRun(RealLifeVideo rlv, int courseNr)
{
    Course course = rlv.courses()[courseNr];
    _run = make_qobject_unique(new Run(_antCentralDispatch, rlv, course));
    _videoWidget.reset(new NewVideoWidget);
    _videoWidget->setRealLifeVideo(rlv);
    _videoWidget->setCourseIndex(courseNr);
    _stackedWidget->addWidget(_videoWidget.data());
    _videoWidget->setSimulation(_run->simulation());

    _stackedWidget->setCurrentIndex(_stackedWidget->indexOf(_videoWidget.data()));
    _savedGeometry = geometry();
    if (isMaximized()) {
        _menuBar->hide();
        showFullScreen();
    }
    connect(&_run->simulation().cyclist(), &Cyclist::distanceChanged, _videoWidget.data(), &NewVideoWidget::setDistance);
    connect(_videoWidget.data(), &NewVideoWidget::readyToPlay, this, [this](bool ready) {
        if (ready) {
            _run->start();
            _videoWidget->displayMessage(tr("Begin cyling to start ride"));
        }
    });
    connect(_run.get(), &Run::newInformationMessage, _videoWidget.data(), &NewVideoWidget::displayInformationBox);
    connect(_run.get(), &Run::stopped, _run.get(), [this]() {
        bool maximize = _videoWidget->isFullScreen();
        _run.reset();
        _stackedWidget->setCurrentIndex(_stackedWidget->indexOf(_listView));
        _stackedWidget->removeWidget(_videoWidget.data());
        _videoWidget.reset();
        _menuBar->show();
        if (maximize) {
            showMaximized();
        } else {
            showNormal();
        }
        setGeometry(_savedGeometry);
        raise();
    });
    connect(_run.get(), &Run::riding, this, &MainWindow::removeDisplayMessage);
    connect(_run.get(), &Run::finished, _run.get(), [this]() {
        const QString text = QString("Ride Finished<br>Finished in %1").arg(_run->time().toString());
        _videoWidget->displayMessage(text);
        QTimer::singleShot(10000, this, SLOT(removeDisplayMessage()));
    });
    connect(_run.get(), &Run::paused, _run.get(), [this]() {
        const QString text = QString("Paused").arg(_run->time().toString());
        _videoWidget->displayMessage(text);
    });
}

bool MainWindow::handleStopRun()
{
    _run->pause();

    QMessageBox stopRunMessageBox(this);
    stopRunMessageBox.setText(tr("Save ride progress before closing?"));
    stopRunMessageBox.setIcon(QMessageBox::Question);
    stopRunMessageBox.setInformativeText(tr("If you don't save the ride, progress will be lost."));
    stopRunMessageBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    stopRunMessageBox.setDefaultButton(QMessageBox::Save);

    switch(stopRunMessageBox.exec()) {
    case QMessageBox::Save:
        _run->saveProgress();
        // fallthrough
    case QMessageBox::Discard:
        _run->stop();
        return true;
    default:
        _run->play();
        return false;
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    // if the user chooses not stop the run, just ignore the event.
    if (_run && !handleStopRun()) {
        event->ignore();
        return;
    }
    qDebug() << "closing main window";
    event->accept();
}

void MainWindow::showEvent(QShowEvent *showEvent)
{
    QWidget::showEvent(showEvent);
    if (!showEvent->spontaneous()) {
        QTimer::singleShot(0, this, SLOT(loadVideos()));
    }
}
