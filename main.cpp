#include <iostream>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <shared_mutex>


// Data structure headers in the ConcurORAM
#include "Block.h" // struct Block defined in this file
#include "TreeNode.h" // struct TreeNode defined in this file
#include "ORAMTree.h" // class ORAMTree defined in this file
#include "Stash.h" // class Stash defined in this file
#include "PositionMap.h" // class PositionMap defined in this file
#include "DRLogSet.h" // class DRLogSet defined in this file
#include "QueryLog.h"
#include "StashSet.h" // class StashSet defined in this file


// parallel header files
#include <thread>
#include <chrono>
#include <memory>
#include <algorithm>
#include <random>
#include <cmath>
#include <iomanip>


using namespace std;




// ORAM Query
class ORAMQuery {
private:
    ORAMTree& tree;
    PositionMap& positionMap;
    Stash& stash;
    DRLogSet& drLogSet;
    QueryLog& queryLog;  

public:
     ORAMQuery(ORAMTree& tree, PositionMap& positionMap, Stash& stash, DRLogSet& drLogSet, QueryLog& queryLog)
        : tree(tree), positionMap(positionMap), stash(stash), drLogSet(drLogSet), queryLog(queryLog) {}

    // Main PathORAM-style Read Operation
     Block read(int blockId)
     {
         auto [queryId, isOverlap] = queryLog.registerQuery(blockId);

         if (isOverlap)
         {
            cout << "Overlapped Block:" << " " << blockId << endl;
             // Dummy read (simulate a random path fetch but ignore result)
             int dummyPath = rand() % (1 << tree.getDepth());
             std::vector<int> pathIndices = tree.getPathIndices(dummyPath);
             for (int idx : pathIndices)
             {
                TreeNode node = tree.getNode(idx);
                for (const Block &b : node.bucket){
                   stash.addBlock(b);
                }
             }

             // Wait until previous queries are done (in real system, check DRL count)
             std::this_thread::sleep_for(std::chrono::milliseconds(100));

             // Try to get it from the DRLogSet
             auto results = drLogSet.readLogSet(blockId);
             for (const auto &b : results){
                if (b.id == blockId)
                return b;
             }

             return Block(-1, "", true); // If still not found
        } else {
            int leafId = positionMap.getPosition(blockId);
            if (leafId == -1)
            {
                Block dummy(-1, "", true);
                drLogSet.writeLogSet(dummy, queryId);
                return dummy;
            }
    
            std::vector<int> pathIndices = tree.getPathIndices(leafId);
            for (int idx : pathIndices)
            {
                TreeNode node = tree.getNode(idx);
                for (const Block &b : node.bucket)
                {
                    stash.addBlock(b);
                }
            }
    
            Block result = stash.fetchBlock(blockId);
            if (result.id == -1)
                result = Block(-1, "", true);
    
            drLogSet.writeLogSet(result, queryId);

            return result;
        }

         
    }
};

void clientQuery(int clientId, int blockId,
            std::shared_ptr<ORAMTree> tree,
            std::shared_ptr<PositionMap> positionMap,
            std::shared_ptr<Stash> stash,
            std::shared_ptr<DRLogSet> drl,
            std::shared_ptr<QueryLog> qlog)
{
    ORAMQuery query(*tree, *positionMap, *stash, *drl, *qlog);
    auto start = std::chrono::high_resolution_clock::now();
    Block result = query.read(blockId);
    auto end = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double, std::milli> latency = end - start;

    std::cout << "Read Result: [ID: " << result.id
    << ", Data: " << result.data
    << ", Dummy: " << (result.isDummy ? "true" : "false") << "]\n";

    std::cout << "Fetch Latency: " << latency.count() << " ms\n";

}


// utility function to print the ORAM tree in ASCII format
void displayORAMtree(const ORAMTree& tree, int depth) {
    int totalNodes = (1 << (depth + 1)) - 1;
    int level = 0;
    int nodesPrinted = 0;

    std::cout << "\n[ORAMTree ASCII Representation]\n";

    while (nodesPrinted < totalNodes) {
        int nodesInLevel = 1 << level;
        int spacing = (1 << (depth - level + 1)); // controls horizontal spacing

        // Print each node at this level
        for (int i = 0; i < nodesInLevel && nodesPrinted < totalNodes; ++i, ++nodesPrinted) {
            TreeNode node = tree.getNode(nodesPrinted);
            std::cout << std::setw(spacing) << "[";
            for (const Block& b : node.bucket) {
                std::cout << b.id << (b.isDummy ? "d" : "") << " ";
            }
            std::cout << "]" << std::setw(spacing);
        }

        std::cout << "\n";
        level++;
    }
}


// utility function to print stash contents

void printStashNamed(const Stash& stash, const std::string& name) {
    std::vector<Block> blocks = stash.getAllBlocks();

    std::cout << "\n[" << name << "]\n";
    if (blocks.empty()) {
        std::cout << "(Empty)\n";
        return;
    }

    for (const Block& b : blocks) {
        std::cout << "  [ID: " << b.id
                  << ", Data: " << b.data
                  << ", Dummy: " << (b.isDummy ? "true" : "false") << "]\n";
    }
}


// utility function to populate the ORAM tree with default blocks
void defaultPopulate(std::shared_ptr<ORAMTree> tree) {
    tree->addBlock(0, Block(0, "Block in root", false));
    tree->addBlock(1, Block(1, "Block in leftmost leaf at depth 1", false));
    tree->addBlock(2, Block(2, "Block in rightmost leaf at depth 1", false));
    tree->addBlock(3, Block(3, "leftmost block at depth 2", false));
    tree->addBlock(4, Block(4, "leftmost + 1 block at depth 2", false));
    tree->addBlock(5, Block(5, "d2 p2", false));
    tree->addBlock(6, Block(6, "d2 rightmost", false));
}

// utility function to populate the PositionMap with default values
void defaultPositionMapPopulate(std::shared_ptr<PositionMap> positionMap) {
    positionMap->updatePosition(3, 0);
    positionMap->updatePosition(4, 1);
    positionMap->updatePosition(5, 2); 
    positionMap->updatePosition(6, 3);
}

int computePathID(int nodeIndex, int depth) {
    int leafStartIndex = (1 << depth) - 1;
    if (nodeIndex < leafStartIndex) {
        std::cerr << "Error: Node index " << nodeIndex << " is not a leaf.\n";
        return -1; // invalid
    }
    return nodeIndex - leafStartIndex;
}



void interactiveMenu(std::shared_ptr<ORAMTree> tree,
                     std::shared_ptr<PositionMap> positionMap,
                     std::shared_ptr<Stash> stash,
                     std::shared_ptr<DRLogSet> drl,
                     std::shared_ptr<QueryLog> qlog,
                     int depth) {
    while (true) {
        std::cout << "\n==== Interactive Menu ====\n";
        std::cout << "1. Read a block from the ORAMTree\n";
        std::cout << "2. Write a block to the ORAMTree\n";
        std::cout << "3. Display ORAMTree\n";
        std::cout << "4. Display contents of Stash\n";
        std::cout << "5. Display contents of PositionMap\n";
        std::cout << "6. Display contents of QueryLog\n";
        std::cout << "7. Display contents of DRLogSet (Current Round)\n";
        std::cout << "8. Exit the program\n";
        std::cout << "Select an option: ";

        int choice;
        std::cin >> choice;

        if (choice == 1) {
            int blockId;
            std::cout << "Enter Block ID to read: ";
            std::cin >> blockId;

            auto start = std::chrono::high_resolution_clock::now();
            clientQuery(1, blockId, tree, positionMap, stash, drl, qlog);
            auto end = std::chrono::high_resolution_clock::now();

            std::chrono::duration<double, std::milli> latency = end - start;
            std::cout << "Fetch Latency: " << latency.count() << " ms\n";

            int leafId = positionMap->getPosition(blockId);
            std::vector<int> path = tree->getPathIndices(leafId);
            std::cout << "Queried Path (Root to Leaf): ";
            for (int idx : path) std::cout << idx << " ";
            std::cout << "\n";

        } else if (choice == 2) {
            int blockId, nodeIndex;
            std::string data;

            std::cout << "Enter Block ID: ";
            std::cin >> blockId;

            std::cin.ignore(); // flush newline
            std::cout << "Enter Block Data: ";
            std::getline(std::cin, data);

            std::cout << "Enter Tree Node Index (0 to " << ((1 << (depth + 1)) - 2) << "): ";
            std::cin >> nodeIndex;

            int pathId = (nodeIndex >= (1 << depth) - 1) ? (nodeIndex - ((1 << depth) - 1)) : -1;
            if (pathId < 0 || pathId >= (1 << depth)) {
                std::cerr << "Error: Invalid leaf node index.\n";
                continue;
            }

            tree->addBlock(nodeIndex, Block(blockId, data, false));
            positionMap->updatePosition(blockId, pathId);
            std::cout << "Block inserted and mapped to path ID " << pathId << ".\n";

        } else if (choice == 3) {
            displayORAMtree(*tree, depth);

        } else if (choice == 4) {
            printStashNamed(*stash, "Stash Contents");

        } else if (choice == 5) {
            positionMap->printMap();

        } else if (choice == 6) {
            qlog->printLog();

        } else if (choice == 7) {
            drl->printCurrentDRL();

        } else if (choice == 8) {
            std::cout << "Exiting the program...\n";
            break;

        } else {
            std::cout << "Invalid choice. Try again.\n";
        }
    }
}


int main()
{
    // === Tree Initialization ===
    int depth;
    int maxConcurrentQueries;
    int numBlocks;

    std::cout << "Enter the depth of the ORAM tree (e.g., 2): ";
    std::cin >> depth;

    std::cout << "Enter the max number of concurrent queries per round (c): ";
    std::cin >> maxConcurrentQueries;

    // Basic input validation
    if (depth < 1 || maxConcurrentQueries < 1) {
        std::cerr << "Error: Depth and c must both be >= 1.\n";
        return 1;
    }

    std::cout << "\nInitialized ORAM tree with depth " << depth
    << " (total nodes: " << ((1 << (depth + 1)) - 1)
    << "), and max " << maxConcurrentQueries << " concurrent queries.\n";

    

    // === ORAM system setup ===
    auto tree = std::make_shared<ORAMTree>(depth);
    auto positionMap = std::make_shared<PositionMap>();
    auto stash = std::make_shared<Stash>();
    auto drl = std::make_shared<DRLogSet>(maxConcurrentQueries);
    auto qlog = std::make_shared<QueryLog>();
    auto stashSet = std::make_shared<StashSet>(maxConcurrentQueries);


    // displayORAMtree(*tree, depth);


    // defaultPopulate(tree);
    // defaultPositionMapPopulate(positionMap);

    interactiveMenu(tree, positionMap, stash, drl, qlog, depth);

    // std::thread t1(clientQuery, 1, 6, tree, positionMap, stash, drl, qlog);
    // std::this_thread::sleep_for(std::chrono::milliseconds(10)); // slight delay
    // std::thread t2(clientQuery, 2, 3, tree, positionMap, stash, drl, qlog);

    // t1.join();
    // t2.join();

    drl->finalizeRound();

    


    // Create and populate a StashSet
    // StashSet stashSet(3);
    // stashSet.addBlockToStash(0, Block(1, "A", false));
    // stashSet.addBlockToStash(0, Block(2, "B", false));
    // stashSet.addBlockToStash(1, Block(3, "C", false));

    // // Print before shuffle
    // for (int i = 0; i < 3; ++i) {
    //     printStashNamed(stashSet.getStash(i), "Stash " + std::to_string(i) + " BEFORE shuffle");
    // }

    // // Shuffle stash 0
    // stashSet.getStash(0).reshuffle();

    // // Print after shuffle
    // for (int i = 0; i < 3; ++i) {
    //     printStashNamed(stashSet.getStash(i), "Stash " + std::to_string(i) + " AFTER shuffle");
    // }

    // // ** PositionMap Printing ***
    // positionMap->printMap();


    return 0;
}
