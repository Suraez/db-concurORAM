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
    Block result = query.read(blockId);

    std::cout << "[Client " << clientId << "] Fetched block: ID = " << result.id
              << ", Data = " << result.data
              << ", Dummy = " << (result.isDummy ? "true" : "false") << std::endl;
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


    // defaultPopulate(tree);
    // defaultPositionMapPopulate(positionMap);



    std::cout << "\nHow many blocks do you want to insert into the tree? ";
    std::cin >> numBlocks;

    for (int i = 0; i < numBlocks; ++i) {
        int blockId, nodeIndex, pathId;
        std::string data;

        std::cout << "\nBlock #" << i + 1 << "\n";

        std::cout << "  Enter Block ID (int): ";
        std::cin >> blockId;

        std::cin.ignore(); // avoid newline issue
        std::cout << "  Enter Block Data (string): ";
        std::getline(std::cin, data);

        std::cout << "  Enter Tree Node Index to insert this block: ";
        std::cin >> nodeIndex;

        pathId = computePathID(nodeIndex, depth);
        if (pathId < 0 || pathId >= (1 << depth)) {
            std::cerr << "  Invalid node index: does not map to a valid leaf.\n";
            continue;  // skip this block
        } else {
            std::cout << "Block ID " << blockId << "mapped to path ID = " << pathId << "\n";
        }


        tree->addBlock(nodeIndex, Block(blockId, data, false));
        positionMap->updatePosition(blockId, pathId);
    }

    

    std::thread t1(clientQuery, 1, 6, tree, positionMap, stash, drl, qlog);
    std::this_thread::sleep_for(std::chrono::milliseconds(10)); // slight delay
    std::thread t2(clientQuery, 2, 3, tree, positionMap, stash, drl, qlog);

    t1.join();
    t2.join();

    drl->finalizeRound();

    displayORAMtree(*tree, depth);



    // **** Stashset Printing ****

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

    // ** PositionMap Printing ***

    positionMap->printMap();


    return 0;
}
