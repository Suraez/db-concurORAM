#include <iostream>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <shared_mutex>

#include "Block.h" // struct Block defined in this file
#include "TreeNode.h" // struct TreeNode defined in this file
#include "ORAMTree.h" // class ORAMTree defined in this file
#include "Stash.h" // class Stash defined in this file
#include "PositionMap.h" // class PositionMap defined in this file
#include "DRLogSet.h" // class DRLogSet defined in this file
using namespace std;




// ORAM Query
class ORAMQuery {
private:
    ORAMTree& tree;
    PositionMap& positionMap;
    Stash& stash;
    DRLogSet& drLogSet;  // 🔄 Inject DRLogSet

public:
    ORAMQuery(ORAMTree& tree, PositionMap& positionMap, Stash& stash, DRLogSet& drLogSet)
        : tree(tree), positionMap(positionMap), stash(stash), drLogSet(drLogSet) {}

    // Main PathORAM-style Read Operation
    Block read(int blockId) {
        int leafId = positionMap.getPosition(blockId);
        if (leafId == -1) {
            Block dummy(-1, "", true);
            drLogSet.appendToCurrent(dummy);
            return dummy;
        }

        std::vector<int> pathIndices = tree.getPathIndices(leafId);

        // Step 1: Read all blocks from the path into the stash
        for (int idx : pathIndices) {
            TreeNode node = tree.getNode(idx);
            for (const Block& b : node.bucket) {
                stash.addBlock(b);
            }
        }

        // Step 2: Try to fetch the target block from the stash
        Block result = stash.fetchBlock(blockId);
        if (result.id == -1) {
            result = Block(-1, "", true); // If not found, return dummy
        }

        // Step 3: Append the fetched block (real or dummy) to the DR-LogSet
        drLogSet.appendToCurrent(result);

        return result;
    }
};

int main() {
    int depth = 2; 
    ORAMTree tree(depth);
    PositionMap positionMap;
    Stash stash;
    DRLogSet drl(2); // Assume max 2 parallel queries per round
    ORAMQuery oramQuery(tree, positionMap, stash, drl);


    Block block1(0, "Block in root", false);
    Block block2(1, "Block in leftmost leaf at depth 1", false);
    Block block3(2, "Block in rightmost leaf at depth 1", false);
    Block block4(3, "leftmost block at depth 2", false);
    Block block5(4, "leftmost + 1 block at depth 2", false);
    Block block6(5, "d2 p2", false);
    Block block7(6, "d2 rightmost", false);

    tree.addBlock(0, block1); 
    tree.addBlock(1, block2);
    tree.addBlock(2, block3);
    tree.addBlock(3, block4);
    tree.addBlock(4, block5);
    tree.addBlock(5, block6);
    tree.addBlock(6, block7);

    // Assign paths (leaf IDs) to each block
    positionMap.updatePosition(3, 0);
    positionMap.updatePosition(4, 1);
    positionMap.updatePosition(5, 2); 
    positionMap.updatePosition(6, 3);

    // Fetch block with ID 5 (assigned to path 2)
    Block fetchedBlock = oramQuery.read(6); 

    cout << "\nFetched block:\n";
    cout << "  Block ID: " << fetchedBlock.id
         << ", Data: " << fetchedBlock.data
         << ", Is Dummy: " << (fetchedBlock.isDummy ? "true" : "false") << endl;

    // Optionally finalize the DRL (after 2 queries for example)
    drl.finalizeRound();

    return 0;
}
