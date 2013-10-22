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
