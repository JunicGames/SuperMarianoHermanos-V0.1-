#pragma once

class Projectile {
public:
    int x, y;
    bool active;
    int dir;

    Projectile() : x(0), y(0), active(false), dir(1) {}
};