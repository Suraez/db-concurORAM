#include <iostream>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <shared_mutex>

using namespace std;

// Block structure
struct Block {
    int id;
    string data;
    bool isDummy;
    Block(int id, string data, bool isDummy) : id(id), data(data), isDummy(isDummy) {}
};

struct TreeNode {  
    vector<Block> bucket; // one node can have multiple blocks
};

class ORAMTree {   // data tree in the paper
private:
    unordered_map<int, TreeNode> tree;
    int depth;
    mutable shared_mutex treeMutex;

public:
    ORAMTree(int depth) : depth(depth) {
        initializeTree();
    }

    void initializeTree() {
        int totalNodes = (1 << (depth + 1)) - 1;
        for (int i = 0; i < totalNodes; ++i) {
            tree[i] = TreeNode();
        }
    }

    void addBlock(int index, const Block& block) {
        unique_lock lock(treeMutex);
        tree[index].bucket.push_back(block);
    }

    TreeNode getNode(int index) {
        shared_lock lock(treeMutex);
        return tree.at(index);
    }
};

// Position Map
class PositionMap {
private:
    unordered_map<int, int> positionMap;
    mutable shared_mutex posMutex;

public:
    void updatePosition(int blockId, int path) {
        unique_lock lock(posMutex);
        positionMap[blockId] = path;
    }

    int getPosition(int blockId) const {
        shared_lock lock(posMutex);
        auto it = positionMap.find(blockId);
        return (it != positionMap.end()) ? it->second : -1;
    }
};

// Stash for temporary storage
class Stash {
private:
    vector<Block> stash;
    mutable shared_mutex stashMutex;

public:
    void addBlock(const Block& block) {
        unique_lock lock(stashMutex);
        stash.push_back(block);
    }

    Block fetchBlock(int id) {
        unique_lock lock(stashMutex);
        for (auto it = stash.begin(); it != stash.end(); ++it) {
            if (it->id == id) {
                Block block = *it;
                stash.erase(it);
                return block;
            }
        }
        return Block(-1, "", true); // Return a dummy block if not found
    }
};

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

// Main function for testing
int main() {
    int depth = 4; // Example tree depth
    ORAMTree tree(depth);
    PositionMap positionMap;
    Stash stash;
    ORAMQuery oramQuery(tree, positionMap, stash);

    // Example: Adding blocks
    Block block(1, "Secret Data", false);
    tree.addBlock(3, block);
    positionMap.updatePosition(1, 3);

    // Example: Reading block
    Block fetchedBlock = oramQuery.read(1);
    if (!fetchedBlock.isDummy) {
        cout << "Read Block: " << fetchedBlock.data << endl;
    } else {
        cout << "Block not found" << endl;
    }
    return 0;
}
