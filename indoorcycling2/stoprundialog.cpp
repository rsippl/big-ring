#include "stoprundialog.h"
#include "ui_stoprundialog.h"

StopRunDialog::StopRunDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::StopRunDialog)
{
    ui->setupUi(this);
}

StopRunDialog::~StopRunDialog()
{
    delete ui;
}

bool StopRunDialog::doesProgressHaveToBeSaved() const
{
    return ui->saveProgressCheckBox->checkState() == Qt::Checked;
}

