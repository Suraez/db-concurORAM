#include "DRLogSet.h"
#include <iostream>

DRLogSet::DRLogSet(int c) : c(c) {}

void DRLogSet::appendToCurrent(const Block& b) {
    currentDRL.push_back(b);
}

// Algorithm 1

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
                if (blockInCurrentDRL)
                {
                    // Sectoin V.A if hte block is in the current DRL and in the bigentry log, then 
                    // dummy block is returned as requests are overlapped for the second client
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

// Algorithm 2

void DRLogSet::writeLogSet(const Block& blk, int queryId) {

    currentDRL.push_back(blk); // appending the block to the current DRL

    // Step 2: Reshuffle the log li if it exists
    if (queryId < static_cast<int>(bigentryLogs.size())) {
        std::vector<Block>& log = bigentryLogs[queryId];  // reading the log li

        std::random_device rd; // seed
        std::mt19937 g(rd()); // random number generator with the seed rd
        std::shuffle(log.begin(), log.end(), g); // shuffling the log li

        // Optional: Print reshuffle action
        std::cout << "[Client " << queryId << "] Reshuffled bigentry log li\n";
    }

    // Step 3: Finalize round if this is the last client (queryId == c - 1)
    if (queryId == c - 1) {
        std::vector<Block> log = currentDRL;

        // Add c dummy blocks
        for (int i = 0; i < c; ++i) {
            log.emplace_back(-1, "", true);
        }

        // Shuffle combined log
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

        std::cout << "[DRL] Finalized query round and created new bigentry log.\n";
    }
}


void DRLogSet::printCurrentDRL() const {
    std::cout << "\n[Current DR-LogSet Contents]\n";
    if (currentDRL.empty()) {
        std::cout << "(Empty)\n";
        return;
    }

    for (const auto& b : currentDRL) {
        std::cout << "  [ID: " << b.id
        << ", Data: " << b.data
        << ", Dummy: " << (b.isDummy ? "true" : "false") << "]\n";
    }
}
