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
}

RlvListWidget::~RlvListWidget()
{
    delete ui;
}

void RlvListWidget::setRealLiveVideos(QList<RealLiveVideo> rlvs)
{
    foreach(const RealLiveVideo& rlv, rlvs) {
        new QListWidgetItem(rlv.name(), ui->rlvList);
    }
    ui->rlvList->setEnabled(true);
}
