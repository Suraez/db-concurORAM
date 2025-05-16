#include "QueryLog.h"
#include <algorithm>

#include <iostream>
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


// Print the contents of the log
void QueryLog::printLog() const {
    std::lock_guard<std::mutex> lock(logMutex);

    std::cout << "\n[QueryLog Contents]\n";
    if (log.empty()) {
        std::cout << "(Log is empty)\n";
        return;
    }

    for (size_t i = 0; i < log.size(); ++i) {
        std::cout << "  Query " << i << ": Block ID = " << log[i] << "\n";
    }
}
