#include "run.h"

#include "antcontroller.h"
#include "cyclist.h"
#include "newvideowidget.h"
#include "simulation.h"

#include <QtCore/QtDebug>

Run::Run(const ANTController& antController, RealLifeVideo& rlv, Course& course, QWidget *parent) :
    QObject(parent), _antController(antController), _rlv(rlv), _course(course),
    _cyclist(new Cyclist(this)), _simulation(new Simulation(*_cyclist, this)), _videoWidget(nullptr)
{
    _simulation->rlvSelected(rlv);
    _simulation->courseSelected(course);

    _videoWidget = new NewVideoWidget(*_simulation);
    _videoWidget->setAttribute(Qt::WA_DeleteOnClose);

    connect(_videoWidget, &NewVideoWidget::destroyed, this, &Run::stopped);
    connect(&antController, SIGNAL(heartRateMeasured(quint8)), _cyclist, SLOT(setHeartRate(quint8)));
    connect(&antController, SIGNAL(cadenceMeasured(quint8)), _cyclist, SLOT(setCadence(quint8)));
    connect(&antController, SIGNAL(powerMeasured(quint16)), _cyclist, SLOT(setPower(quint16)));
}

Run::~Run()
{
    // empty
}

void Run::start()
{
    _videoWidget->setRealLifeVideo(_rlv);
    _videoWidget->setCourse(_course);

    connect(_cyclist, &Cyclist::distanceChanged, _videoWidget, &NewVideoWidget::setDistance);

    connect(_videoWidget, &NewVideoWidget::readyToPlay, _videoWidget, [this](bool ready) {
        if (ready) {
            qDebug() << "starting run";
            this->_cyclist->setPower(300);
            this->_simulation->play(true);
        }
    });

    _videoWidget->showFullScreen();
    _videoWidget->grabKeyboard();
}

void Run::pause()
{
    _simulation->play(false);
}
