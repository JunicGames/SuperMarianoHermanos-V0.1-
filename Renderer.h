#pragma once
#include <ncurses/ncurses.h>
#include <string>
#include <vector>

// En lugar de incluir los .h de las entidades, usa "Forward Declaration"
class MarioPlayer;
class Level;
class Enemy;
class Coin;
class Projectile;
class ScoreManager; // Si no lo necesitas aquí dentro, quita este también

class Renderer {
public:
    // Ahora el compilador sabe que estas clases existen sin tener que cargar todo el archivo .h
    void draw_board(
        const MarioPlayer& mario,
        const Level& level,
        const std::vector<Enemy>& enemies,
        const std::vector<Coin>& coins,
        const std::vector<Projectile>& projectiles
    );

    void draw_hud(const MarioPlayer& mario, int level_time, int game_mode, int hi_score);
    void draw_pause();
    void draw_box(int y, int x, int h, int w);
    void draw_centered(int row, const std::string& str);
};