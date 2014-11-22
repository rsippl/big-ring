#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPushButton>
#include <QTime>
#include <QTimer>

#include "cyclist.h"
#include "reallifevideo.h"
#include "simulation.h"
#include "newvideowidget.h"
#include <QtWidgets/QLabel>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QBoxLayout>
#include <QtWidgets/QSystemTrayIcon>


class ANTController;
class ProfileWidget;
class RealLifeVideoImporter;
class RlvListWidget;

class VideoController;
class VideoWidget;

namespace indoorcycling {
class ScreenSaverBlocker;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT
public:
    explicit
    MainWindow(const RealLifeVideoImporter& parser, Cyclist& cyclist, const ANTController& antController, QWidget *parent = 0);

	void restoreWindow();
	void gotoFullScreen();
signals:
	void importFinished(RealLifeVideoList rlvs);

protected:
	virtual void keyPressEvent(QKeyEvent *);
	virtual void closeEvent(QCloseEvent *);

private slots:
	void rlvSelected(RealLifeVideo rlv);

	void distanceChanged(float distance);
	void slopeChanged(float slope);
	void runTimeChanged(QTime runTime);
	void speedChanged(float speed);

	void hrChanged(quint8 heartRate);
	void powerChanged(float power);
	void cadenceChanged(float cadence);

	void antDeviceFound(QString description);

private:

	QLayout* setUpMain(QWidget *centralWidget);
	QLayout* setupSideBar(QWidget *centralWidget);
	QLabel* createLabel(const QString& text, QColor color, QWidget* centralWidget);

	ProfileWidget* setUpProfileWidget(QWidget*);

	void removeMargins();
	void restoreMargins();

    RealLifeVideo _rlv;
	Cyclist& _cyclist;
	Simulation _simulation;
    NewVideoWidget* videoWidget;
	QListWidget* courseListWidget;
	RlvListWidget* rlvListWidget;
	ProfileWidget* _profileWidget;
	QMargins _margins;
	QRect _cachedGeometry;
	QBoxLayout* _layout;
	QLabel* _distanceLabel;
	QLabel* _slopeLabel;
	QLabel* _speedLabel;
	QLabel* _timeLabel;
	QLabel* _heartRateLabel;
	QLabel* _cadenceLabel;
	QLabel* _powerLabel;
	QPushButton* playButton;

	QSystemTrayIcon* _trayIcon;
    indoorcycling::ScreenSaverBlocker* _screenSaverBlocker;
};

#endif // MAINWINDOW_H
