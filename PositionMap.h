#pragma once

#include <unordered_map>
#include <shared_mutex>

class PositionMap {
private:
    std::unordered_map<int, int> positionMap;
    mutable std::shared_mutex posMutex;

public:
    void updatePosition(int blockId, int path);
    int getPosition(int blockId) const;
};
