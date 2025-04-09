#pragma once
#include <string>

struct Block {
    int id;
    std::string data;
    bool isDummy;

    Block(int id = -1, std::string data = "", bool isDummy = true)
        : id(id), data(std::move(data)), isDummy(isDummy) {}
};
