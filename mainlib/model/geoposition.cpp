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

#include "geoposition.h"

const GeoPosition GeoPosition::NULL_POSITION = GeoPosition();

GeoPosition::GeoPosition():
    GeoPosition(0, QGeoCoordinate())
{
    // empty
}

GeoPosition::GeoPosition(qreal distance, const QGeoCoordinate &coordinate) :
    _distance(distance), _coordinate(coordinate)
{
    // empty
}

qreal GeoPosition::distance() const
{
    return _distance;
}

const QGeoCoordinate &GeoPosition::coordinate() const
{
    return _coordinate;
}

bool GeoPosition::isValid() const
{
    return _coordinate.isValid();
}
