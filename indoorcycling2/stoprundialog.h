#ifndef STOPRUNDIALOG_H
#define STOPRUNDIALOG_H

#include <QDialog>

namespace Ui {
class StopRunDialog;
}

class StopRunDialog : public QDialog
{
    Q_OBJECT

public:
    explicit StopRunDialog(QWidget *parent = 0);
    ~StopRunDialog();
    bool doesProgressHaveToBeSaved() const;
private:
    Ui::StopRunDialog *ui;
};

#endif // STOPRUNDIALOG_H
