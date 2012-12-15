#ifndef MAINWINDOW_H
#define MAINWINDOW_H


#include <QMainWindow>
class QTimer;

#include "reallivevideo.h"

class ANTController;
class RealLiveVideoImporter;
class RlvListWidget;
class QLabel;
class QListWidget;
class QBoxLayout;
class VideoController;
class VideoWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
	explicit MainWindow(const RealLiveVideoImporter& parser, const ANTController& antController, QWidget *parent = 0);

signals:
    void importFinished(RealLiveVideoList rlvs);

protected:
    virtual void keyPressEvent(QKeyEvent *);

private slots:
    void rlvSelected(RealLiveVideo rlv);

	void distanceChanged(float distance);
	void slopeChanged(float slope);
	void hrChanged(quint8 hr);

private:
	QLayout* setUpMain(QWidget *centralWidget);
	QLayout* setupSideBar(QWidget *centralWidget);
	QLabel* createLabel(const QString& text, Qt::GlobalColor color, QWidget* centralWidget);
    void removeMargins();
    void restoreMargins();

	VideoController* videoController;
	VideoWidget* videoWidget;
    QListWidget* courseListWidget;
    RlvListWidget* rlvListWidget;
    QMargins _margins;
    QBoxLayout* _layout;
	QLabel* _distanceLabel;
	QLabel* slopeLabel;
	QLabel* hrLabel;
};

#endif // MAINWINDOW_H
