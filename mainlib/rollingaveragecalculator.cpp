#include "rollingaveragecalculator.h"

RollingAverageCalculator::RollingAverageCalculator(const int durationInMilliseconds):
    _durationInMilliseconds(durationInMilliseconds)
{
}

void RollingAverageCalculator::addValue(const QDateTime &measurementTime, const float value)
{
    _measurements.push_back(std::make_pair(measurementTime, value));
    _sum += value;

    const QDateTime oldestValidTimestamp = measurementTime.addMSecs(-_durationInMilliseconds);
    // search for the first valid
    auto firstValidValueIt = std::find_if(_measurements.begin(), _measurements.end(),
                                          [oldestValidTimestamp](const std::pair<QDateTime,float> &measurement) {
        return measurement.first >= oldestValidTimestamp;
    });
    const float totalRemoved = std::accumulate(_measurements.begin(), firstValidValueIt, 0.0f, [](float accumulator, const std::pair<QDateTime,float> &measurement) {
        return accumulator + measurement.second;
    });
    _sum -= totalRemoved;
    _measurements.erase(_measurements.begin(), firstValidValueIt);

}

float RollingAverageCalculator::currentAverage() const
{
    if (_measurements.empty()) {
        return 0.0f;
    }

    return _sum / _measurements.size();
}
