#pragma once
#include "TreeNode.h"
#include <unordered_map>
#include <shared_mutex>

class ORAMTree {
private:
    std::unordered_map<int, TreeNode> tree;
    int depth;
    mutable std::shared_mutex treeMutex;

public:
    explicit ORAMTree(int depth);
    void initializeTree();
    void addBlock(int index, const Block& block);
    TreeNode getNode(int index);
};
