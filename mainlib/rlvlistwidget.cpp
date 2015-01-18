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

#include "rlvlistwidget.h"
#include "ui_rlvlistwidget.h"

#include <QtDebug>
#include "reallifevideo.h"
RlvListWidget::RlvListWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::RlvListWidget)
{
    ui->setupUi(this);
    ui->rlvList->setDisabled(true);

    connect(ui->rlvList, SIGNAL(currentRowChanged(int)), SLOT(selectionChanged(int)));
}

RlvListWidget::~RlvListWidget()
{
    delete ui;
}

void RlvListWidget::setRealLiveVideos(RealLifeVideoList rlvs)
{
    realLiveVideoList.clear();
    realLiveVideoList = rlvs;
    qDebug() << Q_FUNC_INFO << "rlvs.size() = " << rlvs.size();
    ui->rlvList->clear();
    foreach(const RealLifeVideo& rlv, rlvs) {
		QString rlvInfo = QString("%1 (%2 m)").arg(rlv.name()).arg(rlv.totalDistance());
		new QListWidgetItem(rlvInfo, ui->rlvList);
    }
    ui->rlvList->setEnabled(true);
}

void RlvListWidget::selectionChanged(int row)
{
    if (row == -1 || row >= realLiveVideoList.size())
        emit realLiveVideoSelected(RealLifeVideo());
    else
        emit realLiveVideoSelected(realLiveVideoList[row]);
}
