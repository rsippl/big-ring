#ifndef CREATENEWCOURSEDIALOG_H
#define CREATENEWCOURSEDIALOG_H


#include <QtWidgets/QDialog>
#include "reallifevideo.h"

class QuantityPrinter;
class Thumbnailer;

namespace Ui {
class CreateNewCourseDIalog;
}

class CreateNewCourseDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CreateNewCourseDialog(RealLifeVideo& rlv, QWidget *parent = 0);
    ~CreateNewCourseDialog();

    int startDistanceInMeters() const;
    const QString courseName() const;

private slots:
    void on_startDistanceSlider_valueChanged(int value);

    void on_startDistanceSlider_sliderReleased();

    void on_courseNameEdit_textChanged(const QString &arg1);

private:
    Ui::CreateNewCourseDIalog *ui;
    Thumbnailer* _thumbnailer;
    QuantityPrinter* _quantityPrinter;
    RealLifeVideo _rlv;
    int _startDistanceInMeters;


};

#endif // CREATENEWCOURSEDIALOG_H
