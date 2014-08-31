#include "antmessagegatherer.h"
#include "ANT.h"

AntMessageGatherer::AntMessageGatherer(QObject *parent) :
	QObject(parent)
{
}

void AntMessageGatherer::submitBytes(QByteArray bytes)
{
	_bytesReceived.append(bytes);
	int length;
	QByteArray messageBytes;
	while(!_bytesReceived.isEmpty()) {
		char currentByte = _bytesReceived[0];
		_bytesReceived.remove(0, 1);
		if (messageBytes.isEmpty()) {
			if (currentByte == (char) ANT_SYNC_BYTE) {
				messageBytes.append(currentByte);
			}
		} else {
			if (messageBytes.size() == 1) {
				length = (int) currentByte;
				messageBytes.append(currentByte)
						.append(_bytesReceived.left(2 + length));
				_bytesReceived.remove(0, 2 + length);
				if (messageIsTooShort(messageBytes, length)) {
					_bytesReceived.prepend(messageBytes);
					return;
				} else if (checksumIsOk(messageBytes)) {
					emit antMessageReceived(messageBytes);
				}
				messageBytes.clear();
			}
		}
	}
}


bool AntMessageGatherer::messageIsTooShort(const QByteArray &messageBytes, int length)
{
	// Message length is SYNC_BYTE, LENGTH, TYPE, DATA, CHECKSUM
	return ((4 + length) > messageBytes.length());
}

bool AntMessageGatherer::checksumIsOk(const QByteArray &messageBytes)
{
	quint8 checksum = messageBytes.right(1)[0];
	quint8 calculatedChecksum = 0;
	foreach(const char byte, messageBytes.left(messageBytes.length() - 1)) {
		calculatedChecksum ^= byte;
	}
	return (checksum == calculatedChecksum);
}
