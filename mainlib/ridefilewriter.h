#ifndef RIDEFILEWRITER_H
#define RIDEFILEWRITER_H

#include <QtCore/QObject>

#include "ridefile.h"

class RideFileWriter: public QObject
{
    Q_OBJECT
public:
    explicit RideFileWriter(QObject *parent = 0);
    virtual ~RideFileWriter() {}

    bool writeRideFile(const RideFile &rideFile);
private:
    QString determineFilePath(const RideFile &rideFile);
};

#endif // RIDEFILEWRITER_H
