#include "ridefilewriter.h"

#include <QtCore/QDir>
#include <QtCore/QJsonDocument>
#include <QtCore/QStandardPaths>

RideFileWriter::RideFileWriter(QObject *parent): QObject(parent)
{
    // empty
}

bool RideFileWriter::writeRideFile(const RideFile &rideFile)
{
    const QString filePath = determineFilePath(rideFile);

    QFile outputFile(filePath);
    outputFile.open(QFile::WriteOnly);

    QJsonDocument document(rideFile.toJson());

    outputFile.write(document.toJson(QJsonDocument::Compact));
    return true;
}

QString RideFileWriter::determineFilePath(const RideFile &rideFile)
{
    QDir appDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
    const QString directoryName = QString("Rides/%1").arg(rideFile.rlvName());
    const bool ok = appDir.mkpath(directoryName);
    if (!ok) {
        qFatal("unable to create dir");
    }

    appDir.cd(directoryName);
    const QString filename = QString("%1_%2.ride").arg(rideFile.courseName()).arg(rideFile.startTime().toString(Qt::ISODate));

    return appDir.filePath(filename);
}
