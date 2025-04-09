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

    Block read(int blockId) {
        int path = positionMap.getPosition(blockId);
        if (path == -1) return Block(-1, "", true);

        Block block = stash.fetchBlock(blockId);
        if (block.id != -1) return block;

        TreeNode node = tree.getNode(path);
        for (const Block& b : node.bucket) {
            if (b.id == blockId) {
                return b;
            }
        }
        return Block(-1, "", true);
    }
};

int main() {
    int depth = 4; // binary tree of depth 4
    ORAMTree tree(depth);
    PositionMap positionMap;
    Stash stash;
    ORAMQuery oramQuery(tree, positionMap, stash);

    // after adding a block to the tree, update the position map
    Block block(1, "test", false);
    tree.addBlock(3, block);
    cout << "Adding blocks" << endl;
    positionMap.updatePosition(1, 3);

    tree.addBlock(2, Block(2, "test2", false)); // adding dummy block
    positionMap.updatePosition(2, 3);

    Block fetchedBlock = oramQuery.read(1); 
    cout << "Reading block" << endl; // change block.id to read corresponding block
    if (!fetchedBlock.isDummy) {
        cout << "Block Read: " << fetchedBlock.data << endl;
    } else {
        cout << "Block not found" << endl;
    }
    return 0;
}
