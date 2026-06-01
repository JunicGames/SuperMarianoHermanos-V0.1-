#pragma once

class Coin {
public:
    int x, y;
    bool active;

    Coin(int nx, int ny) : x(nx), y(ny), active(true) {}
};