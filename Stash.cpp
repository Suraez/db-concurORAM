#include "Stash.h"
#include <mutex>  // Required for std::unique_lock and std::shared_mutex
#include <random>
#include <algorithm>

void Stash::addBlock(const Block& block) {
    std::unique_lock lock(stashMutex);
    stash.push_back(block);
}

Block Stash::fetchBlock(int id) {
    std::unique_lock lock(stashMutex);
    for (auto it = stash.begin(); it != stash.end(); ++it) {
        if (it->id == id) {
            Block block = *it;
            stash.erase(it);
            return block;
        }
    }
    return Block(-1, "", true); // Return dummy block if not found
}

bool Stash::contains(int id) const {
    std::shared_lock lock(stashMutex);
    for (const auto& block : stash) {
        if (block.id == id) return true;
    }
    return false;
}

void Stash::clear() {
    std::unique_lock lock(stashMutex);
    stash.clear();
}

std::vector<Block> Stash::getAllBlocks() const {
    std::shared_lock lock(stashMutex);
    return stash; // Return a copy
}


void Stash::reshuffle() {
    std::unique_lock lock(stashMutex);
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(stash.begin(), stash.end(), g);
}