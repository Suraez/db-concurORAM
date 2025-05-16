#include "PositionMap.h"
#include <mutex>  // Required for std::unique_lock and std::shared_mutex    
#include <iostream>

using namespace std;
void PositionMap::updatePosition(int blockId, int path) {
    std::unique_lock lock(posMutex);
    positionMap[blockId] = path;
}

int PositionMap::getPosition(int blockId) const {
    std::shared_lock lock(posMutex);
    auto it = positionMap.find(blockId);
    cout << "PositionMap::getPosition: blockId = " << blockId << endl;
    return (it != positionMap.end()) ? it->second : -1; // it->second is the path
}

void PositionMap::printMap() const {
    std::shared_lock lock(posMutex);

    std::cout << "\n[PositionMap Contents]\n";
    if (positionMap.empty()) {
        std::cout << "(PositionMap is empty)\n";
        return;
    }

    for (const auto& [blockId, path] : positionMap) {
        std::cout << "  Block ID: " << blockId << " → Path: " << path << "\n";
    }
}
