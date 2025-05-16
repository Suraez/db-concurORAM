#pragma once

#include "Block.h"
#include <vector>
#include <algorithm>
#include <random>

class DRLogSet {
private:
    int c; // Max number of parallel clients per round
    std::vector<Block> currentDRL;  // one query round (i.e. two users requesting same data block)
    // is equal to one currentDRL
    std::vector<std::vector<Block>> bigentryLogs; //vector of vectors
    // stores the blocks that have been previously queried
    std::vector<std::vector<int>> searchIndices;

public:
    explicit DRLogSet(int c);

    void appendToCurrent(const Block& b);

    std::vector<Block> readLogSet(int blockId);

    void finalizeRound(); // Called after c queries
    void writeLogSet(const Block& blk, int queryId);
    void printCurrentDRL() const;

};