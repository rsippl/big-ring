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
#ifndef VIDEODETAILSWIDGET_H
#define VIDEODETAILSWIDGET_H

#include <QtWidgets/QLabel>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QWidget>
#include "reallifevideo.h"

class Thumbnailer;
class AltitudeProfileWidget;
class QuantityPrinter;
class VideoScreenshotWidget;

class VideoDetailsWidget : public QWidget
{
    Q_OBJECT
public:
    explicit VideoDetailsWidget(QWidget *parent = 0);

signals:
    void playClicked(RealLifeVideo& rlv, int courseNr);
public slots:
    void setVideo(RealLifeVideo& rlv);
protected:
    virtual void paintEvent(QPaintEvent *);
private:
    QWidget* setupDetails();
    QWidget* setupCourseList();
    QWidget* setupVideoScreenshot();
    QWidget* setupProfileLabel();

    void updateVideoScreenshotLabel(const RealLifeVideo& rlv, const qreal distance, QPixmap& pixmap);

    RealLifeVideo _currentRlv;
    int _courseIndex;
    Thumbnailer* _thumbnailer;
    QLabel* _nameLabel;
    QLabel* _distanceLabel;
    VideoScreenshotWidget* _videoScreenshotWidget;
    AltitudeProfileWidget* _profileLabel;
    QListWidget* _courseListWidget;
    QuantityPrinter* _quantityPrinter;
};

#endif // VIDEODETAILSWIDGET_H
