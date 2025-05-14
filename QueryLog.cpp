#include "QueryLog.h"
#include <algorithm>
std::pair<int, bool> QueryLog::registerQuery(int blockId) {
    std::lock_guard<std::mutex> lock(logMutex);

    bool overlap = (std::find(log.begin(), log.end(), blockId) != log.end());
    log.push_back(blockId);
    return {static_cast<int>(log.size() - 1), overlap}; // index, overlap
}

void QueryLog::clear() {
    std::lock_guard<std::mutex> lock(logMutex);
    log.clear();
}

size_t QueryLog::size() {
    std::lock_guard<std::mutex> lock(logMutex);
    return log.size();
}

