#include "ORAMTree.h"
#include <mutex>  // Required for std::unique_lock and std::shared_mutex
#include <iostream>
using namespace std;
#include <algorithm>  // for std::reverse

ORAMTree::ORAMTree(int depth) : depth(depth) {
    initializeTree();
}

void ORAMTree::initializeTree() {
    int totalNodes = (1 << (depth + 1)) - 1; // depth starts from 0 therefore total nodes = 2^(depth+1) - 1
    cout << "Total nodes in the tree: " << totalNodes << endl;
    for (int i = 0; i < totalNodes; ++i) {
        tree[i] = TreeNode();
    }
}

void ORAMTree::addBlock(int index, const Block& block) {
    std::unique_lock lock(treeMutex);
    tree[index].bucket.push_back(block);
}

TreeNode ORAMTree::getNode(int index) const {
    std::shared_lock lock(treeMutex);
    return tree.at(index);
}


// Returns node indices from root to the given leaf ID
std::vector<int> ORAMTree::getPathIndices(int leafId) {
    std::vector<int> path;
    int index = leafId + (1 << depth) - 1; // index of leaf node in array representation
    while (index >= 0) {
        path.push_back(index);
        if (index == 0) break;
        index = (index - 1) / 2; // go to parent
    }
    std::reverse(path.begin(), path.end()); // from root to leaf
    return path;
}


int ORAMTree::getDepth() const {
    return depth;
}
