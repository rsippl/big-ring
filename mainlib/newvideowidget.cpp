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

#include "newvideowidget.h"

#include <functional>
#include <cmath>

#include <QtCore/QtDebug>
#include <QtCore/QPropertyAnimation>
#include <QtCore/QUrl>
#include <QtOpenGL/QGLWidget>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGraphicsDropShadowEffect>
#include <QtGui/QResizeEvent>


#include "clockgraphicsitem.h"
#include "informationboxgraphicsitem.h"
#include "messagepanelitem.h"
#include "profileitem.h"
#include "sensoritem.h"
#include "simulation.h"
#include "screensaverblocker.h"
#include "videoplayer.h"


NewVideoWidget::NewVideoWidget(QWidget *parent) :
    QGraphicsView(parent), _informationBoxHideTimer(new QTimer(this)),
    _screenSaverBlocker(new indoorcycling::ScreenSaverBlocker(this)),
    _mouseIdleTimer(new QTimer(this))
{
    setMinimumSize(800, 600);
    setFocusPolicy(Qt::StrongFocus);
    QGLWidget* viewPortWidget = new QGLWidget(QGLFormat(QGL::SampleBuffers));
    setViewport(viewPortWidget);
    setFrameShape(QFrame::NoFrame);

    setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    QGraphicsScene* scene = new QGraphicsScene(this);
    setScene(scene);

    setSizeAdjustPolicy(QGraphicsView::AdjustIgnored);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    setViewportUpdateMode(QGraphicsView::MinimalViewportUpdate);

    addClock(scene);
    addInformationBox(scene);
    addMessagePanel(scene);
    addSensorItems(scene);

    _profileItem = new ProfileItem;
    scene->addItem(_profileItem);

    setupVideoPlayer(viewPortWidget);

    _pausedItem = new QGraphicsTextItem;
    QFont bigFont;
    bigFont.setPointSize(36);
    _pausedItem->setFont(bigFont);
    _pausedItem->setDefaultTextColor(Qt::white);
    _pausedItem->setPlainText("Paused");

    _informationBoxHideTimer->setInterval(10000);
    _informationBoxHideTimer->setSingleShot(true);
    connect(_informationBoxHideTimer, &QTimer::timeout,
            _informationBoxItem, [this]() {
        _informationBoxItem->hide();
    });
    _mouseIdleTimer->setInterval(500);
    _mouseIdleTimer->setSingleShot(true);
    connect(_mouseIdleTimer, &QTimer::timeout, _mouseIdleTimer, []() {
        QApplication::setOverrideCursor(Qt::BlankCursor);
    });
}

void NewVideoWidget::setupVideoPlayer(QGLWidget* paintWidget)
{
    _videoPlayer = new VideoPlayer(paintWidget, this);

    connect(_videoPlayer, &VideoPlayer::videoLoaded, this, [this](qint64 numberOfFrames) {
        qDebug() << "total number of frames" << numberOfFrames;
        this->_rlv.setNumberOfFrames(numberOfFrames);
        if (this->_course.isValid()) {
            this->seekToStart(_course);
        }
    });
    connect(_videoPlayer, &VideoPlayer::seekDone, this, [this]() {
        emit readyToPlay(true);
    });
    connect(_videoPlayer, &VideoPlayer::updateVideo, this, [this]() {
        this->viewport()->update(rect());
    });
}

void NewVideoWidget::addClock(QGraphicsScene* scene)
{
    _clockItem = new ClockGraphicsItem;
    scene->addItem(_clockItem);
}

void NewVideoWidget::addInformationBox(QGraphicsScene *scene)
{
    _informationBoxItem = new InformationBoxGraphicsItem;
    _informationBoxItem->hide();
    scene->addItem(_informationBoxItem);
}

void NewVideoWidget::addMessagePanel(QGraphicsScene *scene)
{
    _messagePanelItem = new MessagePanelItem;
    _messagePanelItem->hide();
    scene->addItem(_messagePanelItem);
}

NewVideoWidget::~NewVideoWidget()
{
    // empty
}

bool NewVideoWidget::isReadyToPlay()
{
    return _videoPlayer->isReadyToPlay();
}

void NewVideoWidget::setRealLifeVideo(RealLifeVideo rlv)
{
    Q_EMIT(readyToPlay(false));
    _rlv = rlv;
    _profileItem->setRlv(rlv);
    _videoPlayer->stop();
    _videoPlayer->loadVideo(rlv.videoFilename());
}

void NewVideoWidget::setCourse(Course &course)
{
    _course = course;
    _profileItem->setCourse(course);
    seekToStart(_course);
}

void NewVideoWidget::setCourseIndex(int index)
{
    if (!_rlv.isValid()) {
        return;
    }
    Course course = _rlv.courses()[qMax(0, index)];
    setCourse(course);
}

void NewVideoWidget::setDistance(float distance)
{
    _videoPlayer->stepToFrame(_rlv.frameForDistance(distance));
}

void NewVideoWidget::displayMessage(const QString &message)
{
    if (message.isEmpty()) {
        _messagePanelItem->hide();
    } else {
        _messagePanelItem->setMessage(message);
        QRectF rect = _messagePanelItem->boundingRect();
        rect.moveCenter(QPointF(sceneRect().width() / 2, sceneRect().height() / 2));

        _messagePanelItem->setPos(rect.topLeft());
        _messagePanelItem->show();
    }
}

void NewVideoWidget::displayInformationBox(const InformationBox &informationBox)
{
    _informationBoxItem->setInformationBox(informationBox);

    QRectF rect = _informationBoxItem->boundingRect();
    rect.moveCenter(QPointF(sceneRect().width() / 2, 3 * sceneRect().height() / 4));
    rect.moveBottom(sceneRect().height() * 27 / 32);
    _informationBoxItem->setPos(rect.topLeft());
    _informationBoxItem->show();
    // a previous information box was perhaps popped up. By stopping the time, we make sure
    // it does not hide the new one when the timer times out.
    _informationBoxHideTimer->stop();
    _informationBoxHideTimer->start();
}

void NewVideoWidget::setSimulation(const Simulation& simulation)
{
    connect(&simulation, &Simulation::runTimeChanged, _clockItem, &ClockGraphicsItem::setTime);

    const Cyclist& cyclist = simulation.cyclist();
    _profileItem->setCyclist(&cyclist);
    connect(&cyclist, &Cyclist::powerChanged, this, [this](int power) {
        _wattageItem->setValue(QVariant::fromValue(power));
    });
    connect(&cyclist, &Cyclist::cadenceChanged, this, [this](int cadence) {
        _cadenceItem->setValue(QVariant::fromValue(cadence));
    });
    connect(&cyclist, &Cyclist::speedChanged, this, [this](float speed) {
        _speedItem->setValue(QVariant::fromValue(speed));
    });
    connect(&simulation, &Simulation::slopeChanged, this, [this](float grade) {
        _gradeItem->setValue(QVariant::fromValue(grade));
    });
    connect(&cyclist, &Cyclist::heartRateChanged, this, [this](int heartRate) {
        _heartRateItem->setValue(QVariant::fromValue(heartRate));
    });
    connect(&cyclist, &Cyclist::distanceChanged, this, [this](float distance) {
        float distanceDifference = _course.end() - distance;
        _distanceItem->setValue(QVariant::fromValue(std::fabs(distanceDifference)));
    });

}

void NewVideoWidget::goToFullscreen()
{
    showFullScreen();
}

void NewVideoWidget::resizeEvent(QResizeEvent *resizeEvent)
{
    setSceneRect(viewport()->rect());
    QRectF clockItemRect = _clockItem->boundingRect();
    clockItemRect.moveCenter(QPointF(sceneRect().width() / 2, 0.0));
    clockItemRect.moveTop(0.0);
    _clockItem->setPos(clockItemRect.topLeft());


    QPointF scenePosition = mapToScene(width() / 2, 0);
    scenePosition = mapToScene(0, height() /8);
    _wattageItem->setPos(scenePosition);
    scenePosition = mapToScene(0, 2* height() /8);
    _heartRateItem->setPos(scenePosition);
    scenePosition = mapToScene(0, 3 * height() /8);
    _cadenceItem->setPos(scenePosition);
    scenePosition = mapToScene(width(), 1 * height() / 8);
    _speedItem->setPos(scenePosition.x() - _speedItem->boundingRect().width(), scenePosition.y());
    scenePosition = mapToScene(width(), 2 * height() /8);
    _distanceItem->setPos(scenePosition.x() - _distanceItem->boundingRect().width(), scenePosition.y());
    scenePosition = mapToScene(width(), 3 * height() / 8);
    _gradeItem->setPos(scenePosition.x() - _gradeItem->boundingRect().width(), scenePosition.y());
    resizeEvent->accept();

    QPointF center = mapToScene(viewport()->rect().center());
    _pausedItem->setPos(center);

    _profileItem->setGeometry(QRectF(mapToScene(resizeEvent->size().width() * 1 / 16, resizeEvent->size().height() * 27 / 32), QSizeF(sceneRect().width() * 7 / 8, sceneRect().height() * 1 / 8)));

}

void NewVideoWidget::enterEvent(QEvent *)
{
    QApplication::setOverrideCursor(Qt::BlankCursor);
    _mouseIdleTimer->start();
}

void NewVideoWidget::leaveEvent(QEvent *)
{
    QApplication::setOverrideCursor(Qt::ArrowCursor);
    _mouseIdleTimer->stop();
}

void NewVideoWidget::mouseMoveEvent(QMouseEvent *)
{
    QApplication::setOverrideCursor(Qt::ArrowCursor);
    _mouseIdleTimer->stop();
    _mouseIdleTimer->start();
}

void NewVideoWidget::closeEvent(QCloseEvent *)
{
    QApplication::setOverrideCursor(Qt::ArrowCursor);
}

/*!
 *
 * \brief Draw background by telling the video sink to update the complete background.
 * \param painter the painter used for drawing.
 */
void NewVideoWidget::drawBackground(QPainter *painter, const QRectF &)
{
    QPointF topLeft = mapToScene(viewport()->rect().topLeft());
    QPointF bottemRight = mapToScene(viewport()->rect().bottomRight());
    const QRectF r = QRectF(topLeft, bottemRight);
    _videoPlayer->displayCurrentFrame(painter, r, Qt::KeepAspectRatioByExpanding);

}

void NewVideoWidget::seekToStart(Course &course)
{
    quint32 frame = _rlv.frameForDistance(course.start());
    _videoPlayer->seekToFrame(frame);
}

void NewVideoWidget::addSensorItems(QGraphicsScene *scene)
{
    _wattageItem = new SensorItem(QuantityPrinter::Quantity::Power);
    scene->addItem(_wattageItem);
    _heartRateItem = new SensorItem(QuantityPrinter::Quantity::HeartRate);
    scene->addItem(_heartRateItem);
    _cadenceItem = new SensorItem(QuantityPrinter::Quantity::Cadence);
    scene->addItem(_cadenceItem);
    _speedItem = new SensorItem(QuantityPrinter::Quantity::Speed);
    scene->addItem(_speedItem);
    _distanceItem = new SensorItem(QuantityPrinter::Quantity::Distance);
    scene->addItem(_distanceItem);
    _gradeItem = new SensorItem(QuantityPrinter::Quantity::Grade);
    scene->addItem(_gradeItem);
}

