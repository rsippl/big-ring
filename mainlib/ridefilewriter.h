#ifndef RIDEFILEWRITER_H
#define RIDEFILEWRITER_H

#include <QtCore/QFile>
#include <QtCore/QObject>
#include <QtCore/QXmlStreamWriter>

#include "ridefile.h"

class RideFileWriter: public QObject
{
    Q_OBJECT
public:
    explicit RideFileWriter(QObject *parent = 0);
    virtual ~RideFileWriter() {}

    bool writeRideFile(const RideFile &rideFile);
private:
    QString determineFilePath(const RideFile &rideFile) const;

    void writeXml(const RideFile &rideFile, QFile &outputFile);
    void writeLapSummary(QXmlStreamWriter &writer, const RideFile &rideFile);
    void writeTrackPoint(QXmlStreamWriter &writer, const QDateTime &startTime, const RideFile::Sample &sample);

    QString dateTimeString(const QDateTime dateTime, bool withMillis = false) const;
};

#endif // RIDEFILEWRITER_H
