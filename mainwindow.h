#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QTime>
#include <QTimer>

#include "cyclist.h"
#include "reallivevideo.h"
#include "simulation.h"

#include <QLabel>
#include <QListWidget>
#include <QBoxLayout>
#include <QSystemTrayIcon>

class ANTController;
class RealLiveVideoImporter;
class RlvListWidget;
class VideoController;
class VideoWidget;

class MainWindow : public QMainWindow
{
	Q_OBJECT
public:
	explicit MainWindow(const RealLiveVideoImporter& parser, Cyclist& cyclist, const ANTController& antController, QWidget *parent = 0);

	void restoreWindow();
	void gotoFullScreen();
signals:
	void importFinished(RealLiveVideoList rlvs);

protected:
	virtual void keyPressEvent(QKeyEvent *);

private slots:
	void rlvSelected(RealLiveVideo rlv);

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
	void removeMargins();
	void restoreMargins();

	Cyclist& _cyclist;
	Simulation _simulation;
	VideoWidget* videoWidget;
	VideoController* videoController;
	QListWidget* courseListWidget;
	RlvListWidget* rlvListWidget;
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
};

#endif // MAINWINDOW_H
