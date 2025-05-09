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

using namespace std;




// ORAM Query
class ORAMQuery {
private:
    ORAMTree& tree;
    PositionMap& positionMap;
    Stash& stash;

public:
    ORAMQuery(ORAMTree& tree, PositionMap& positionMap, Stash& stash) 
        : tree(tree), positionMap(positionMap), stash(stash) {}


    // ORAMQuery first tries to find the path where the block is stored using the position map.
    // If the block is not found in the PositionMap, it returns the dummy block.


    // if the block is found in the position map, it fetches the block from the stash.
    std::vector<Block> read(int blockId) {
        int leafId = positionMap.getPosition(blockId);
        if (leafId == -1) return { Block(-1, "", true) };
    
        std::vector<int> pathIndices = tree.getPathIndices(leafId);
        std::vector<Block> pathBlocks;
    
        for (int idx : pathIndices) {
            TreeNode node = tree.getNode(idx);
            for (const Block& b : node.bucket) {
                stash.addBlock(b);
                pathBlocks.push_back(b);
            }
        }
    
        // Optionally remap the block ID and update position map here.
    
        return pathBlocks;  // return all blocks read along the path
    }
    
    
};

int main() {
    int depth = 1; // binary tree of depth 1 → 2 leaves, 3 nodes
    ORAMTree tree(depth);
    PositionMap positionMap;
    Stash stash;
    ORAMQuery oramQuery(tree, positionMap, stash);

    cout << "Adding blocks" << endl;

    // Add two blocks to the rightmost path (path ID 1, which is root [0] -> right child [2])
    Block block1(1, "Block in root", false);
    Block block2(2, "Block in left leaf", false);
    Block block3(3, "Block in right leaf", false);
    tree.addBlock(0, block1); 
    tree.addBlock(1, block2);// Add to root
    tree.addBlock(2, block3); // Add to right leaf (index 2)

    // Both blocks are assigned to path ID 1 (rightmost path)
    positionMap.updatePosition(1, 1);
    positionMap.updatePosition(2, 1);

    // Now query block 1 (will fetch root → right leaf = path ID 1)
    std::vector<Block> fetchedBlocks = oramQuery.read(2); // blockId = 1, assigned to path 1

    cout << "\nFetched blocks on path to leaf 1:\n";
    for (const Block& b : fetchedBlocks) {
        cout << "  Block ID: " << b.id 
             << ", Data: " << b.data 
             << ", Is Dummy: " << (b.isDummy ? "true" : "false") << endl;
    }

    return 0;
}