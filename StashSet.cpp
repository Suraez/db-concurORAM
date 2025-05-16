#include "StashSet.h"
#include <random>
#include <algorithm>
#include <iostream>

StashSet::StashSet(int numClients) {
for (int i = 0; i < numClients; ++i) {
    tempStashes.push_back(std::make_unique<Stash>());
}  
}

void StashSet::addBlockToStash(int stashIndex, const Block& block) {
    if (stashIndex < tempStashes.size()) {
        tempStashes[stashIndex]->addBlock(block); 
    }
}


std::vector<Block> StashSet::readStashSet(int id, int queryId) {
    std::vector<Block> result;
    bool found = false;

    for (int j = 0; j < tempStashes.size(); ++j) {
        std::vector<Block> blocks = tempStashes[j]->getAllBlocks();
        bool stashHasBlock = false;

        for (const Block& b : blocks) {
            if (b.id == id) {
                stashHasBlock = true;
                if (!found) {
                    result.push_back(b);
                    found = true;
                } else {
                    result.emplace_back(-1, "", true);
                }
                break;
            }
        }

        if (!stashHasBlock) {
            result.emplace_back(-1, "", true);
        }
    }

    if (queryId < tempStashes.size()) {
        tempStashes[queryId]->reshuffle();  
    }

    return result;
}


void StashSet::clear() {
    for (auto& stash : tempStashes) {
        stash->clear(); 
    }
}

Stash& StashSet::getStash(int index) {
    return *tempStashes.at(index);  // * becuase of the unique_ptr in the StashSet class
}

