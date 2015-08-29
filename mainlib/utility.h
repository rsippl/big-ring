#ifndef UTILITY_H
#define UTILITY_H

#include <functional>
#include <map>
#include <utility>
#include <vector>

namespace indoorcycling
{

template <typename T, typename U>
std::map<U,T> toMap(const std::vector<T> &items, std::function<U(T)> &keyCreatorFunction)
{
    std::map<U,T> targetMap;
    std::transform(items.begin(), items.end(), std::inserter(targetMap, targetMap.begin()),
                   [keyCreatorFunction](const T &item) {
        return std::make_pair(keyCreatorFunction(item), item);
    });
    return targetMap;
}

}
#endif // UTILITY_H
