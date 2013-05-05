#ifndef BYTEARRAYTOANTMESSAGECONVERTER_H
#define BYTEARRAYTOANTMESSAGECONVERTER_H

#include <QObject>

class AntMessageGatherer : public QObject
{
	Q_OBJECT
public:
	explicit AntMessageGatherer(QObject *parent = 0);
	
public slots:
	void submitBytes(QByteArray bytes);
signals:
	void antMessageReceived(QByteArray messageBytes);
private:
	bool messageIsTooShort(const QByteArray &messageBytes, int length);
	bool checksumIsOk(const QByteArray& messageBytes);
	
	QByteArray _bytesReceived;
};

#endif // BYTEARRAYTOANTMESSAGECONVERTER_H
