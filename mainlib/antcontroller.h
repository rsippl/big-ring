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
	Q_PROPERTY(quint8 cadence READ cadence NOTIFY cadenceMeasured)
	Q_PROPERTY(quint16 power READ power NOTIFY powerMeasured)
public:
	explicit ANTController(QObject *parent = 0);
	virtual ~ANTController();
	
	quint8 heartRate() const;
	quint16 power() const;
	quint8 cadence() const;
signals:
	void heartRateMeasured(quint8 bpm);
	void powerMeasured(quint16 power);
	void cadenceMeasured(quint8 cadence);

	void deviceFound(QString description);

public slots:
	void heartRateReceived(quint8 bpm);
	void powerReceived(float power);
	void cadenceReceived(float cadence);

private slots:
	void foundDevice(int channel, int device, int device_type, QString description, QString typeCode);
private:
	void initialize();

	quint8 _heartRate;
	quint16 _power;
	quint8 _cadence;

	QThread* antThread;
	ANT* const ant;
	QTimer* const antTimer;
};

#endif // ANTCONTROLLER_H
