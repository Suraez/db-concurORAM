#pragma once

#include "Block.h"
#include <vector>
#include <shared_mutex>

class Stash {
private:
    std::vector<Block> stash;
    mutable std::shared_mutex stashMutex;

public:
    void addBlock(const Block& block);
    Block fetchBlock(int id);
    bool contains(int id) const;
    void clear();
    std::vector<Block> getAllBlocks() const;
};
