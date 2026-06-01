#include "Constantes.h"
#pragma once

class MarioPlayer {

public:
    int x, y;
    int vy;        // velocidad vertical
    bool on_ground;
    int state;     // 0=normal, 1=super, 2=fire
    bool invincible;
    int lives;
    int score;
    int coins;
    bool has_fire;
    int dir;       // 1=derecha, -1=izquierda

    MarioPlayer() {
        reset();
    }

    void reset() {
        x         = 5;
        y         = GROUND_ROW - 1;
        vy        = 0;
        on_ground = true;
        state     = 0;
        invincible= false;
        lives     = 3;
        score     = 0;
        coins     = 0;
        has_fire  = false;
        dir       = 1;
    }
};