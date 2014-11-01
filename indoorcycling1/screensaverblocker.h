#ifndef SCREENSAVERBLOCKER_H
#define SCREENSAVERBLOCKER_H

#include <QtCore/QObject>
#include <QtWidgets/QWidget>

#ifdef Q_OS_LINUX
#include <QtCore/QProcess>
#endif

namespace indoorcycling
{
class ScreenSaverBlocker : public QObject
{
    Q_OBJECT
public:
    explicit ScreenSaverBlocker(QWidget* window, QObject *parent = 0);
    virtual ~ScreenSaverBlocker();

public slots:
    void handleError(QProcess::ProcessError error);

private:
    void blockScreenSaver();

    QWidget* const _window;
#ifdef Q_OS_LINUX
    QProcess _process;
#endif


};
}
#endif // SCREENSAVERBLOCKER_H
