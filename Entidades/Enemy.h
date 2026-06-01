#pragma once

class Enemy {
public:
    int x, y;
    bool alive;
    int dir;       // -1 izquierda, 1 derecha
    int type;      // 0=Goomba, 1=Koopa, 2=Shell

    Enemy(int nx, int ny, int ntype, int ndir) 
        : x(nx), y(ny), alive(true), dir(ndir), type(ntype) {}
};