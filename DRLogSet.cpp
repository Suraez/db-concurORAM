#include "DRLogSet.h"

DRLogSet::DRLogSet(int c) : c(c) {}

void DRLogSet::appendToCurrent(const Block& b) {
    currentDRL.push_back(b);
}

std::vector<Block> DRLogSet::readLogSet(int blockId) {
    std::vector<Block> result;

    // First, check current DRL
    for (const Block& b : currentDRL) {
        if (b.id == blockId) {
            result.push_back(b);
            return result;
        }
    }

    // Check past bigentry logs
    for (size_t i = 0; i < bigentryLogs.size(); ++i) {
        const auto& log = bigentryLogs[i];
        auto& index = searchIndices[i];
        bool found = false;

        for (const auto& b : log) {
            if (b.id == blockId && std::find(index.begin(), index.end(), blockId) != index.end()) {
                result.push_back(b);
                index.erase(std::remove(index.begin(), index.end(), blockId), index.end());
                found = true;
                break;
            }
        }

        if (!found) {
            result.emplace_back(-1, "", true); // Dummy block
        }
    }

    return result;
}

void DRLogSet::finalizeRound() {
    if (currentDRL.empty()) return;

    std::vector<Block> log = currentDRL;

    // Add c dummy blocks
    for (int i = 0; i < c; ++i) {
        log.emplace_back(-1, "", true);
    }

    // Shuffle the log
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(log.begin(), log.end(), g);

    bigentryLogs.push_back(log);

    // Build search index
    std::vector<int> index;
    for (const auto& b : currentDRL) {
        if (!b.isDummy) {
            index.push_back(b.id);
        }
    }

    searchIndices.push_back(index);
    currentDRL.clear();
}
