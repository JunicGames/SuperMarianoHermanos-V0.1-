#pragma once
#include <thread>
#include <mutex>
#include <cmath>
#include <unistd.h>

#include "Entidades/MarioPlayer.h"
#include "Constantes.h"
#include "Level.h"
#include "Renderer.h"
#include "ScoreManager.h"

class Game {
private:
    MarioPlayer mario;
    Level level;
    Renderer renderer;
    ScoreManager scores;

    std::vector<Enemy> enemies;
    std::vector<Coin> coins;
    std::vector<Projectile> projectiles;

    int game_mode;
    bool game_running;
    bool game_paused;
    int level_time;

    std::mutex board_mutex;

    void init_game();
    void timer_loop();
    void enemies_loop();
    void projectiles_loop();
    void update_mario(int key);
    void result_screen(bool won);

public:
    Game();
    void game_loop();
    void set_mode(int mode);
};