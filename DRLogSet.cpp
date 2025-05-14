#include "DRLogSet.h"

DRLogSet::DRLogSet(int c) : c(c) {}

void DRLogSet::appendToCurrent(const Block& b) {
    currentDRL.push_back(b);
}

std::vector<Block> DRLogSet::readLogSet(int blockId) {  // 1st algorithm
    std::vector<Block> result;

    bool blockInCurrentDRL = false;
    for (const Block& b : currentDRL) {
        if (b.id == blockId) {
            blockInCurrentDRL = true;
            result.push_back(b); // still read from current DRL first
            break;
        }
    }


    // Check past bigentry logs 
    // Bigentry logs are logs containing  past query round’s shuffled result blocks
    // searchIndices basically gives you the indices of the blocks in the bigentry logs

    for (int i = static_cast<int>(bigentryLogs.size()) - 1; i >= 0; --i)
    {
        const auto &log = bigentryLogs[i];
        auto &index = searchIndices[i];
        bool found = false;

        for (const auto &b : log)
        {
            if (b.id == blockId && std::find(index.begin(), index.end(), blockId) != index.end())
            {
                // 🔍 Check if also in DRL → Case 2
                if (blockInCurrentDRL)
                {
                    // Sectoin V.A if hte block is in the current DRL and in the bigentry log, then reurn
                    // dummy block
                    result.emplace_back(-1, "", true);
                }
                else
                {
                    // Return the actual block (case 1)
                    result.push_back(b);
                }
                // Remove from search index regardless
                index.erase(std::remove(index.begin(), index.end(), blockId), index.end());
                found = true;
                break;
            }
        }

        if (!found)
        {
            // Case 3: return dummy block if it is not found in the bigentry log
            result.emplace_back(-1, "", true);
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
