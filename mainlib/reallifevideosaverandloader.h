/*
 * Copyright (c) 2015 Ilja Booij (ibooij@gmail.com)
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
#ifndef REALLIFEVIDEOSAVERANDLOADER_H
#define REALLIFEVIDEOSAVERANDLOADER_H

#include <QtCore/QObject>
#include <memory>
#include "reallifevideo.h"

class RealLifeVideoSaverAndLoader : public QObject
{
    Q_OBJECT
public:
    explicit RealLifeVideoSaverAndLoader(QObject *parent = 0);

    std::unique_ptr<RealLifeVideo> load(const QString &fileName);
public slots:
    void saveRlv(const QString &filename, const RealLifeVideo &rlv);

private:
    QString absoluteFilenameForRlv(const QString &name) const;
    void saveCourses(QDataStream &out, const std::vector<Course> &courses) const;
    std::vector<Course> readCourses(QDataStream &in) const;

    void saveDistanceMappings(QDataStream &out, const std::vector<DistanceMappingEntry> &distanceMappings) const;
    std::vector<DistanceMappingEntry> readDistanceMappings(QDataStream &in) const;

    void saveProfile(QDataStream &out, const Profile &profile) const;
    Profile readProfile(QDataStream &in);

    void saveProfileEntries(QDataStream &out, const std::vector<ProfileEntry> &entries) const;
    std::vector<ProfileEntry> readProfileEntries(QDataStream &in) const;

    void saveInformationBoxes(QDataStream &out, const std::vector<InformationBox> &entries) const;
    std::vector<InformationBox> readInformationBoxes(QDataStream &in) const;
};

#endif // REALLIFEVIDEOSAVERANDLOADER_H
