#include "PositionMap.h"
#include <mutex>  // Required for std::unique_lock and std::shared_mutex    

void PositionMap::updatePosition(int blockId, int path) {
    std::unique_lock lock(posMutex);
    positionMap[blockId] = path;
}

int PositionMap::getPosition(int blockId) const {
    std::shared_lock lock(posMutex);
    auto it = positionMap.find(blockId);
    return (it != positionMap.end()) ? it->second : -1;
}
