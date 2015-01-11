#ifndef RUN_H
#define RUN_H

#include <QtCore/QObject>

#include "reallifevideo.h"

class ANTController;
class Cyclist;
class Simulation;
class NewVideoWidget;

class Run : public QObject
{
    Q_OBJECT
public:
    explicit Run(const ANTController& antController, RealLifeVideo& rlv, Course& course, QWidget *parent = 0);
    virtual ~Run();
signals:
    void stopped();
public slots:
    void start();
    void pause();

private:
    const ANTController& _antController;
    RealLifeVideo _rlv;
    Course _course;
    Cyclist* _cyclist;
    Simulation* _simulation;
    NewVideoWidget* _videoWidget;
};

#endif // RUN_H
