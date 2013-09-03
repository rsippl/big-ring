#ifndef ANTCONTROLLER_H
#define ANTCONTROLLER_H

#include <QObject>
#include <QThread>
#include <QTimer>


class ANT;

class ANTController : public QObject
{
	Q_OBJECT
	Q_PROPERTY(quint8 heartRate READ heartRate NOTIFY heartRateMeasured)
public:
	explicit ANTController(QObject *parent = 0);
	virtual ~ANTController();
	
	quint8 heartRate() const;
signals:
	void heartRateMeasured(quint8 bpm);
	void powerMeasured(float power);
	void cadenceMeasured(float cadence);

	void deviceFound(QString description);

	void finished();
public slots:
	void quit();
	void threadFinished();
	void heartRateReceived(quint8 bpm);
private slots:
	void foundDevice(int channel, int device, int device_type, QString description, QString typeCode);
private:
	void initialize();

	quint8 _bpm;

	QThread* antThread;
	ANT* const ant;
	QTimer* const antTimer;
};

#endif // ANTCONTROLLER_H
