#pragma once
#include <vector>

class Level {
public:
    std::vector<std::vector<int>> platforms = {
        {23,  0, 79},   // suelo principal
        {18, 10, 20},
        {15, 25, 38},
        {12, 45, 58},
        {18, 60, 72},
        {10, 30, 40},
    };

    std::vector<std::vector<int>> pipes = {
        {14, 15},
        {14, 30},
        {14, 47},
        {11, 35}
    };

    std::vector<std::vector<int>> qblocks = {
        {20, 5},
        {20, 68}
    };
    std::vector<int> qblock_state;
    
};