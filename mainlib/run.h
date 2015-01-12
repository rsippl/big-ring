#ifndef RUN_H
#define RUN_H

#include <QtCore/QObject>

#include "reallifevideo.h"

class ANTController;
class Simulation;
class NewVideoWidget;

class Run : public QObject
{
    Q_OBJECT
public:
    explicit Run(const ANTController& antController, Simulation *simulation, RealLifeVideo& rlv, Course& course,
                 NewVideoWidget *displayWidget = 0, QObject *parent = 0);
    virtual ~Run();

    bool isRunning() const;
signals:
    void stopped();
public slots:
    void start();
    void stop();
    void pause();

private:
    const ANTController& _antController;
    RealLifeVideo _rlv;
    Course _course;
    Simulation* _simulation;
    NewVideoWidget* _videoWidget;
    bool _running;
};

#endif // RUN_H
