#pragma once

#include <vector>
#include <mutex>

class QueryLog {
private:
    std::vector<int> log;
    mutable std::mutex logMutex;


public:
    // Register a block ID, return index (query ID), and overlap status
    std::pair<int, bool> registerQuery(int blockId);

    // Clear log (used after a round is complete)
    void clear();

    size_t size();

    void printLog() const;


};
