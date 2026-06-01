#include <ncurses/ncurses.h>
#include <ctime>
#include <cstdlib>
#include <iostream> // Necesario para la limpieza

#include "Game.h"
#include "Menu.h"

int main()
{
    // Limpieza agresiva de la terminal antes de iniciar ncurses
    std::cout << "\033[2J\033[H"; 
    
    initscr();
    raw(); 
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
    nodelay(stdscr, TRUE);

    if (has_colors()) {
        start_color();
        init_pair(1, COLOR_WHITE,   COLOR_BLACK);
        init_pair(2, COLOR_GREEN,   COLOR_BLACK);
        init_pair(3, COLOR_YELLOW,  COLOR_BLACK);
        init_pair(4, COLOR_RED,     COLOR_BLACK);
        init_pair(5, COLOR_CYAN,    COLOR_BLACK);
        init_pair(6, COLOR_BLACK,   COLOR_WHITE);
        init_pair(7, COLOR_BLUE,    COLOR_BLACK);
        init_pair(8, COLOR_MAGENTA, COLOR_BLACK);
        bkgd(COLOR_PAIR(1));
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
        } else if (opt == 1) {
            menu.instructions_screen();
        } else if (opt == 2) {
            menu.scores_screen();
        } else {
            break;
        }
    }

    endwin();
    return 0;
}