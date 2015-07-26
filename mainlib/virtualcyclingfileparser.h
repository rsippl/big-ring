#ifndef VIRTUALCYCLINGFILEPARSER_H
#define VIRTUALCYCLINGFILEPARSER_H

#include "reallifevideo.h"
#include <memory>
#include <QtCore/QObject>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QXmlStreamReader>

namespace indoorcycling {

namespace virtualcyclingfileparser {
class ProfileEntry;
}
class VirtualCyclingFileParser : public QObject
{
    Q_OBJECT
public:
    explicit VirtualCyclingFileParser(const QList<QFileInfo>& videoFiles, QObject *parent = 0);

    std::unique_ptr<RealLifeVideo> parseVirtualCyclingFile(QFile &inputFile) const;
private:
    std::unique_ptr<RealLifeVideo> parseXml(QXmlStreamReader& reader) const;

    QList<ProfileEntry> convertProfileEntries(const QList<virtualcyclingfileparser::ProfileEntry> virtualCyclingProfileEntries) const;
    QList<virtualcyclingfileparser::ProfileEntry> readProfileEntries(QXmlStreamReader &reader) const;
    QString findVideoFile(QString filename) const;
    QList<Course> readCourses(QXmlStreamReader& reader) const;

    const QList<QFileInfo> _videoFilesInfo;
};
}
#endif // VIRTUALCYCLINGFILEPARSER_H
