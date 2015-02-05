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
#include "thumbnailer.h"
#include "videoscreenshotlabel.h"

VideoDetailsWidget::VideoDetailsWidget(QWidget *parent) :
    QWidget(parent), _profilePainter(new ProfilePainter(this)), _thumbnailer(new Thumbnailer(this))
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    setLayout(layout);

    QHBoxLayout* topLayout = new QHBoxLayout(this);
    topLayout->addWidget(setupVideoScreenshot(), 2);
    topLayout->addWidget(setupDetails(), 1);

    layout->addLayout(topLayout);
    layout->addWidget(setupProfileLabel());


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
    _profileLabel->setPixmap(_profilePainter->paintProfile(rlv, _profileLabel->rect()));
}

QWidget *VideoDetailsWidget::setupDetails()
{
    QGroupBox* detailsGroupBox = new QGroupBox(tr("Details"), this);
    QVBoxLayout* layout = new QVBoxLayout(this);

    _nameLabel = new QLabel;
    layout->addWidget(_nameLabel);
    _distanceLabel = new QLabel;
    layout->addWidget(_distanceLabel);
    QPushButton* startButton = new QPushButton(tr("Start"));
    layout->addWidget(startButton);
    connect(startButton, &QPushButton::clicked, startButton, [this](){
        emit playClicked(_currentRlv);
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

QWidget *VideoDetailsWidget::setupProfileLabel()
{
    _profileLabel = new QLabel;
    _profileLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    _profileLabel->setScaledContents(true);
    return _profileLabel;
}

void VideoDetailsWidget::updateVideoScreenshotLabel(const RealLifeVideo &rlv, QPixmap &pixmap)
{
    RealLifeVideo thisRlv = rlv;
    if (thisRlv == _currentRlv) {
        _videoScreenshotLabel->setPixmap(pixmap);
    }
}
