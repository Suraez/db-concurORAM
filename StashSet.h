#pragma once

#include "Stash.h"
#include "Block.h"
#include <vector>
#include <memory>

class StashSet {
private:
    std::vector<std::unique_ptr<Stash>> tempStashes;
public:
    StashSet(int numClients);  // initialize with c stashes
    std::vector<Block> readStashSet(int id, int queryId);
    void addBlockToStash(int stashIndex, const Block& block);
    void clear();
    Stash& getStash(int index);

};
