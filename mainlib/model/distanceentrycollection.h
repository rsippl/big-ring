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

#ifndef DISTANCEENTRYCOLLECTION_H
#define DISTANCEENTRYCOLLECTION_H

#include <functional>
#include <vector>

template <typename T>
class DistanceEntryCollection {
public:
    typedef typename std::vector<T>::const_iterator It;

    explicit DistanceEntryCollection(const std::vector<T> &entries, const std::function<qreal(const T&)> distanceFunction);

    const typename std::vector<T>::const_iterator iteratorForDistance(const qreal distance);
    const T *entryForDistance(const qreal distance);

private:
    std::vector<T> _entries;
    const std::function<qreal(const T&)> _distanceFunction;

    typename std::vector<T>::const_iterator _currentEntry;
};

template <typename T>
DistanceEntryCollection<T>::DistanceEntryCollection(const std::vector<T> &entries, const std::function<qreal (const T &)> distanceFunction):
    _entries(entries), _distanceFunction(distanceFunction), _currentEntry(_entries.begin())
{
    // empty
}

template <typename T>
const typename std::vector<T>::const_iterator DistanceEntryCollection<T>::iteratorForDistance(const qreal distance)
{
    if (_entries.empty()) {
        return _entries.end();
    }
    const bool atLastEntry = _currentEntry == _entries.end() || _currentEntry + 1 == _entries.end();
    const bool isDistanceSmallerThenCurrent = distance < _distanceFunction(*_currentEntry);
    const bool isDistanceBiggerThenCurrent = !atLastEntry && distance > _distanceFunction(*(_currentEntry + 1));

    // if the distance we're looking for is smaller than the current entry, or bigger than the
    // start of the next entry, then we need to search which entry to use. If not, we can
    // simply use the current entry.
    if (isDistanceSmallerThenCurrent || (!atLastEntry && isDistanceBiggerThenCurrent)) {
        auto it = std::lower_bound(_entries.begin(), _entries.end(), distance, [this](const T &entry, qreal distance) {
            return _distanceFunction(entry) < distance;
        });

        // lower bound gives us the first entry that is not smaller then distance, so we need to
        // go back one step, unless we're already at the beginning.
        if (it != _entries.begin() && (it == _entries.end() || _distanceFunction(*it) > distance) ) {
            it--;
        }
        _currentEntry = it;
    }
    return _currentEntry;
}

template <typename T>
const T *DistanceEntryCollection<T>::entryForDistance(const qreal distance)
{
    auto it = iteratorForDistance(distance);
    if (it == _entries.end()) {
        return nullptr;
    }
    return &(*it);
}

#endif // DISTANCEENTRYCOLLECTION_H
