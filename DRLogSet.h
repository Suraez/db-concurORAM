#pragma once

#include "Block.h"
#include <vector>
#include <algorithm>
#include <random>

class DRLogSet {
private:
    int c; // Max number of parallel clients per round
    std::vector<Block> currentDRL;
    std::vector<std::vector<Block>> bigentryLogs;
    std::vector<std::vector<int>> searchIndices;

public:
    explicit DRLogSet(int c);

    void appendToCurrent(const Block& b);

    std::vector<Block> readLogSet(int blockId);

    void finalizeRound(); // Called after c queries
};