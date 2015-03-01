/*
 * Copyright (c) 2012-2015 Ilja Booij (ibooij@gmail.com)
 *
 * This file is part of Big Ring Indoor Video Cycling
 *
 * Big Ring Indoor Video Cycling is free software: you can redistribute
 * it and/or modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * Big Ring Indoor Video Cycling  is distributed in the hope that it will
 * be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with Big Ring Indoor Video Cycling.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include "reallifevideoimporter.h"
#include "rlvfileparser.h"

#include <functional>

#include <QtCore/QDirIterator>
#include <QtCore/QSettings>

#include <QStringList>
#include <QtConcurrent/QtConcurrentMap>
#include <QtConcurrent/QtConcurrentRun>
#include <QtDebug>

namespace
{
RealLifeVideo parseRealLiveVideoFile(QFile &rlvFile, const QList<QFileInfo>& videoFiles);
QList<QFileInfo> findFiles(const QString& root, const QString& pattern);
QList<QFileInfo> findRlvFiles(QString& root);

RealLifeVideoList importRlvFiles(QString root);
}

RealLifeVideoImporter::RealLifeVideoImporter(QObject* parent): QObject(parent)
{
    // empty
}

void RealLifeVideoImporter::parseRealLiveVideoFilesFromDir(const QString &root)
{
    QFutureWatcher<RealLifeVideoList> *futureWatcher = new QFutureWatcher<RealLifeVideoList>();
    connect(futureWatcher, &QFutureWatcher<RealLifeVideoList>::finished, futureWatcher, [=]() {
        importReady(futureWatcher->future().result());
        futureWatcher->deleteLater();
    });

    QFuture<QList<RealLifeVideo> > importRlvFuture = QtConcurrent::run(importRlvFiles, root);
    futureWatcher->setFuture(importRlvFuture);
}

void RealLifeVideoImporter::importReady(const RealLifeVideoList &rlvs)
{
    RealLifeVideoList validRlvs;

    for (auto rlv: rlvs) {
        if (rlv.isValid() && rlv.type() == ProfileType::SLOPE) {
            validRlvs.append(rlv);
        }
    }
    // sort rlv list by name
    qSort(validRlvs.begin(), validRlvs.end(), RealLifeVideo::compareByName);

    emit importFinished(validRlvs);
}

namespace
{
RealLifeVideo parseRealLiveVideoFile(QFile &rlvFile, const QList<QFileInfo>& videoFiles)
{
    RlvFileParser parser(videoFiles);
    RealLifeVideo rlv = parser.parseRlvFile(rlvFile);
    QSettings settings;
    settings.beginGroup("unfinished_runs");
    const QString key = rlv.name();
    if (settings.contains(key)) {
        float distance = settings.value(key).toFloat();
        rlv.setUnfinishedRun(distance);
    }
    settings.endGroup();
    settings.beginGroup(QString("%1.custom_courses").arg(rlv.name()));
    QStringList customCourseNames = settings.allKeys();
    for (QString customCourseName: customCourseNames) {
        int startDistance = settings.value(customCourseName).toInt();
        rlv.addStartPoint(startDistance, customCourseName);
    }
    settings.endGroup();
    return rlv;
}

QList<QFileInfo> findFiles(const QString& root, const QString& pattern)
{
    QStringList filters;
    filters << pattern;
    QDirIterator it(root, filters, QDir::NoFilter, QDirIterator::Subdirectories);

    QList<QFileInfo> filePaths;
    while(it.hasNext()) {
        it.next();
        filePaths.append(it.fileInfo());
    }
    return filePaths;
}

QList<QFileInfo> findRlvFiles(QString& root)
{
    return findFiles(root, "*.rlv");
}

RealLifeVideoList importRlvFiles(QString root)
{
    const QList<QFileInfo> rlvFiles = findRlvFiles(root);
    const QList<QFileInfo> aviFiles = findFiles(root, "*.avi");

    std::function<RealLifeVideo(const QFileInfo&)> importFunction([aviFiles](const QFileInfo& fileInfo) -> RealLifeVideo {
        QFile file(fileInfo.canonicalFilePath());
        return parseRealLiveVideoFile(file, aviFiles);
    });

    return QtConcurrent::mapped(rlvFiles.begin(), rlvFiles.end(), importFunction).results();
}

}
