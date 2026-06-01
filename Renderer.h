#pragma once
#include <ncurses/ncurses.h>

#include "Constantes.h"
#include "ScoreManager.h"
#include "Entidades/MarioPlayer.h"
#include "Entidades/Coin.h"
#include "Entidades/Enemy.h"
#include "Entidades/Projectile.h"
#include "Level.h"

class Renderer {
public:
    void draw_board(
        const MarioPlayer& mario,
        const Level& level,
        const std::vector<Enemy>& enemies,
        const std::vector<Coin>& coins,
        const std::vector<Projectile>& projectiles
    );

    void draw_hud(
        const MarioPlayer& mario,
        int level_time,
        int game_mode,
        int hi_score
    );

    void draw_pause();
    void draw_box(int y, int x, int h, int w);
    void draw_centered(int row, const std::string& str);
};