/*
 * mario.cpp - Super Mario Bros. ASCII - Fase 2: Entorno Gráfico (Migrado a C++)
 * Universidad del Valle de Guatemala
 * Programación de Microprocesadores - Primer Ciclo 2026
 *
 * Compilar con: g++ mario.cpp -o mario -lncurses
 * Requiere: ncurses (soporta hilos estándar de C++11)
 */

#include <ncurses/ncurses.h>
#include <ctime>
#include <cstdlib>

#include "Game.h"
#include "Menu.h"

int main()
{
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
    nodelay(stdscr, TRUE);

    if (has_colors()) {
        start_color();
        use_default_colors();

        init_pair(1, COLOR_WHITE,  -1);
        init_pair(2, COLOR_GREEN,  -1);
        init_pair(3, COLOR_YELLOW, -1);
        init_pair(4, COLOR_RED,    -1);
        init_pair(5, COLOR_CYAN,   -1);
        init_pair(6, COLOR_BLACK,  COLOR_WHITE);
        init_pair(7, COLOR_BLUE,   -1);
    }

    std::srand(static_cast<unsigned>(std::time(nullptr)));

    Game super_mario;
    Menu menu;

    while (true) {
        int opt = menu.menu_screen();

        if (opt == 0) {
            int mode = menu.mode_select_screen();

            if (mode > 0) {
                super_mario.set_mode(mode);
                super_mario.game_loop();
            }
        }
        else if (opt == 1) {
            menu.instructions_screen();
        }
        else if (opt == 2) {
            menu.scores_screen();
        }
        else {
            break;
        }
    }

    endwin();
    return 0;
}