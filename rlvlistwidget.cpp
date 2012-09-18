#include "rlvlistwidget.h"
#include "ui_rlvlistwidget.h"

#include <QtDebug>
#include "reallivevideo.h"
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

void RlvListWidget::setRealLiveVideos(RealLiveVideoList rlvs)
{
    realLiveVideoList.clear();
    realLiveVideoList = rlvs;
    qDebug() << Q_FUNC_INFO << "rlvs.size() = " << rlvs.size();
    ui->rlvList->clear();
    foreach(const RealLiveVideo& rlv, rlvs) {
		QString rlvInfo = QString("%1 (%2)").arg(rlv.name()).arg(rlv.videoInformation().frameRate());
		new QListWidgetItem(rlvInfo, ui->rlvList);
    }
    ui->rlvList->setEnabled(true);
}

void RlvListWidget::selectionChanged(int row)
{
    if (row == -1 || row >= realLiveVideoList.size())
        emit realLiveVideoSelected(RealLiveVideo());
    else
        emit realLiveVideoSelected(realLiveVideoList[row]);
}
