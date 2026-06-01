#pragma once
#include <vector>
#include "HighScore.h"

class ScoreManager {
private:
    std::vector<HighScore> high_scores;
public:
    void add_score(const std::string& name, int score);
    std::vector<HighScore>& get_scores();
};