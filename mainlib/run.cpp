#include "run.h"

#include "antcontroller.h"
#include "cyclist.h"
#include "newvideowidget.h"
#include "simulation.h"

#include <QtCore/QtDebug>

Run::Run(const ANTController& antController, Simulation *simulation, RealLifeVideo& rlv, Course& course,
         NewVideoWidget *videoWidget, QObject* parent) :
    QObject(parent), _antController(antController), _rlv(rlv), _course(course),
    _simulation(simulation), _videoWidget(videoWidget)
{
    _simulation->rlvSelected(rlv);
    _simulation->courseSelected(course);

    connect(&antController, SIGNAL(heartRateMeasured(quint8)), &_simulation->cyclist(), SLOT(setHeartRate(quint8)));
    connect(&antController, SIGNAL(cadenceMeasured(quint8)), &_simulation->cyclist(), SLOT(setCadence(quint8)));
    connect(&antController, SIGNAL(powerMeasured(quint16)), &_simulation->cyclist(), SLOT(setPower(quint16)));
}

Run::~Run()
{
    // empty
}

bool Run::isRunning() const
{
    return _running;
}

void Run::start()
{
    _videoWidget->setRealLifeVideo(_rlv);
    _videoWidget->setCourse(_course);

    connect(&_simulation->cyclist(), &Cyclist::distanceChanged, _videoWidget, &NewVideoWidget::setDistance);

    connect(_videoWidget, &NewVideoWidget::readyToPlay, this, [this](bool ready) {
        if (ready) {
            qDebug() << "starting run";
            _simulation->cyclist().setPower(300);
            this->_simulation->play(true);
        }
    });
    _running = true;
}

void Run::stop()
{
    _simulation->play(false);
    emit stopped();
}

void Run::pause()
{
    _simulation->play(false);
}
