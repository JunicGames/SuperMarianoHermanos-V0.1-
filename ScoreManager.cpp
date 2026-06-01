#include "ScoreManager.h"

void ScoreManager::add_score(const std::string& name, int score)
{
    high_scores.push_back({name, score});
}

std::vector<HighScore>& ScoreManager::get_scores()
{
    return high_scores;
}