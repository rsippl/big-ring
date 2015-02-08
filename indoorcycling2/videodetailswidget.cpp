/*
 * Copyright (c) 2015 Ilja Booij (ibooij@gmail.com)
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
#include "videodetailswidget.h"

#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QPushButton>

#include "profilepainter.h"
#include "profilewidget.h"
#include "thumbnailer.h"
#include "videoscreenshotlabel.h"

VideoDetailsWidget::VideoDetailsWidget(QWidget *parent) :
    QWidget(parent), _profilePainter(new ProfilePainter(this)), _thumbnailer(new Thumbnailer(this))
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    setLayout(layout);

    QHBoxLayout* topLayout = new QHBoxLayout(this);
    topLayout->addWidget(setupDetails(), 1);
    topLayout->addWidget(setupVideoScreenshot(), 2);

    layout->addLayout(topLayout, 2);
    layout->addWidget(setupProfileLabel(), 1);


    connect(_thumbnailer, &Thumbnailer::pixmapUpdated, _thumbnailer, [this](const RealLifeVideo& rlv, QPixmap pixmap){
        updateVideoScreenshotLabel(rlv, pixmap);
    });
}

void VideoDetailsWidget::setVideo(RealLifeVideo &rlv)
{
    _currentRlv = rlv;
    _nameLabel->setText(rlv.name());
    _distanceLabel->setText(QString("%1 km").arg(QString::number(rlv.totalDistance() / 1000, 'f', 1)));
    _videoScreenshotLabel->setPixmap(_thumbnailer->thumbnailFor(rlv));
    _profileLabel->setVideo(rlv);

    _courseListWidget->clear();
    for (const Course& course: rlv.courses()) {
        new QListWidgetItem(course.name(), _courseListWidget);
    }
    _courseListWidget->setCurrentRow(0);
}

QWidget *VideoDetailsWidget::setupDetails()
{
    QGroupBox* detailsGroupBox = new QGroupBox(tr("Details"), this);
    QVBoxLayout* layout = new QVBoxLayout(this);

    _nameLabel = new QLabel;
    layout->addWidget(_nameLabel);
    _distanceLabel = new QLabel;
    layout->addWidget(_distanceLabel);
    layout->addWidget(setupCourseList());
    QPushButton* startButton = new QPushButton(tr("Start"));
    layout->addWidget(startButton);
    connect(startButton, &QPushButton::clicked, startButton, [this](){
        emit playClicked(_currentRlv, _courseListWidget->currentRow());
    });

    detailsGroupBox->setLayout(layout);
    return detailsGroupBox;
}

QWidget *VideoDetailsWidget::setupVideoScreenshot()
{
    _videoScreenshotLabel = new VideoScreenshotLabel;
    _videoScreenshotLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    return _videoScreenshotLabel;
}

QWidget *VideoDetailsWidget::setupCourseList()
{
    _courseListWidget = new QListWidget(this);
    connect(_courseListWidget, &QListWidget::currentRowChanged, _courseListWidget, [this](int row) {
        _profileLabel->setCourseIndex(row);
        if (row >= 0) {
            qDebug() << "course selected:" << _currentRlv.courses()[row].name();
        }
    });

    return _courseListWidget;
}

QWidget *VideoDetailsWidget::setupProfileLabel()
{
    _profileLabel = new ProfileWidget;
    _profileLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    return _profileLabel;
}

void VideoDetailsWidget::updateVideoScreenshotLabel(const RealLifeVideo &rlv, QPixmap &pixmap)
{
    RealLifeVideo thisRlv = rlv;
    if (thisRlv == _currentRlv) {
        _videoScreenshotLabel->setPixmap(pixmap);
    }
}
