#include "ORAMTree.h"
#include <mutex>  // Required for std::unique_lock and std::shared_mutex


ORAMTree::ORAMTree(int depth) : depth(depth) {
    initializeTree();
}

void ORAMTree::initializeTree() {
    int totalNodes = (1 << (depth + 1)) - 1;
    for (int i = 0; i < totalNodes; ++i) {
        tree[i] = TreeNode();
    }
}

void ORAMTree::addBlock(int index, const Block& block) {
    std::unique_lock lock(treeMutex);
    tree[index].bucket.push_back(block);
}

TreeNode ORAMTree::getNode(int index) {
    std::shared_lock lock(treeMutex);
    return tree.at(index);
}
