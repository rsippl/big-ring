#ifndef ANTCONTROLLER_H
#define ANTCONTROLLER_H

#include <QObject>
#include <QThread>
#include <QTimer>


class ANT;

class ANTController : public QObject
{
	Q_OBJECT
public:
	explicit ANTController(QObject *parent = 0);
	virtual ~ANTController();
	
signals:
	void heartRateMeasured(quint8 bpm);
	void powerMeasured(float power);
	void cadenceMeasured(float cadence);

	void deviceFound(QString description);
public slots:
	
private slots:
	void foundDevice(int channel, int device, int device_type, QString description, QString typeCode);
private:
	void initialize();

	QThread* antThread;
	ANT* const ant;
	QTimer* const antTimer;
};

#endif // ANTCONTROLLER_H
