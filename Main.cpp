#include <iostream>
#include <ncurses/ncurses.h>
#include <vector>
#include <string>

void mostrar_menu(int seleccion) {
    clear();
    mvprintw(2, 25, "=============================");
    mvprintw(3, 25, "    SUPER MARIO BROS - UVG   ");
    mvprintw(4, 25, "=============================");
    
    std::vector<std::string> opciones = {
        "1. Iniciar Partida",
        "2. Ver Instrucciones",
        "3. Puntajes Destacados",
        "4. Salir"
    };

    for(size_t i = 0; i < opciones.size(); ++i) {
        if(static_cast<int>(i) == seleccion) {
            attron(A_REVERSE); // Resalta la opción seleccionada
            mvprintw(7 + i, 30, "%s", opciones[i].c_str());
            attroff(A_REVERSE);
        } else {
            mvprintw(7 + i, 30, "%s", opciones[i].c_str());
        }
    }
    refresh();
}

void mostrar_instrucciones() {
    clear();
    mvprintw(2, 5, "=== INSTRUCCIONES ===");
    mvprintw(4, 5, "Objetivo: Avanza de izquierda a derecha evadiendo enemigos y llega a la meta.");
    mvprintw(6, 5, "Controles:");
    mvprintw(7, 7, "[Flecha Izquierda / Derecha] : Moverse");
    mvprintw(8, 7, "[Espacio / Flecha Arriba]    : Saltar");
    mvprintw(9, 7, "[A]                          : Lanzar Fuego");
    mvprintw(10, 7, "[P]                          : Pausa");
    mvprintw(11, 7, "[Q / ESC]                    : Salir");
    
    mvprintw(13, 5, "Elementos Visuales (ASCII):");
    mvprintw(14, 7, "(^>) : Mario Normal      [?] : Bloque Sorpresa");
    mvprintw(15, 7, "/mm\\ : Goomba            [#] : Bloque Ladrillo");
    
    mvprintw(18, 5, "Presione cualquier tecla para regresar al menu...");
    getch();
}

void mostrar_puntajes() {
    clear();
    mvprintw(2, 20, "=== TOP HIGH SCORES ===");
    mvprintw(5, 25, "1. MARIO_UVG  - 9999 pts");
    mvprintw(6, 25, "2. LUIGI_COMP - 7500 pts");
    mvprintw(7, 25, "3. PEACH_SYS  - 5000 pts");
    
    mvprintw(12, 20, "Presione cualquier tecla para regresar al menu...");
    getch();
}

void iniciar_juego() {
    // Renderizado del mapa inicial del juego (80x24)
    clear();
    nodelay(stdscr, TRUE); // Entrada no bloqueante requerida
    
    int mario_x = 5, mario_y = 18;
    int ch;
    
    while(true) {
        clear();
        // Dibujar HUD
        mvprintw(0, 2, "SCORE: 000100   LIVES: 3   COINS: 05   TIME: 400   MODE: LENTO");
        mvprintw(1, 0, "--------------------------------------------------------------------------------");
        
        // Dibujar un escenario estático básico (Suelo en filas 20-22)
        for(int x=0; x<80; x++) {
            mvprintw(20, x, "=");
            mvprintw(21, x, "#");
        }
        
        // Bloques y elementos de prueba
        mvprintw(14, 30, "[?]");
        mvprintw(14, 33, "[#]");
        mvprintw(20, 50, "/mm\\"); // Un Goomba estático
        
        // Dibujar a Mario
        mvprintw(mario_y, mario_x, "(^>)");
        
        refresh();
        napms(50); // usleep equivalente para fluidez
        
        ch = getch();
        if (ch == 'q' || ch == 'Q' || ch == 27) { // Salir con Q o ESC
            break;
        }
        // Movimiento básico de entorno para la demo gráfica
        if (ch == KEY_RIGHT && mario_x < 75) mario_x++;
        if (ch == KEY_LEFT && mario_x > 0) mario_x--;
        
    }
    nodelay(stdscr, FALSE);
}

int main() {
    initscr();
    clear();
    noecho();
    cbreak();
    keypad(stdscr, TRUE);
    curs_set(0); // Ocultar cursor físico de la terminal

    int seleccion = 0;
    int tecla;

    while(true) {
        mostrar_menu(seleccion);
        tecla = getch();

        if(tecla == KEY_UP) {
            seleccion = (seleccion - 1 + 4) % 4;
        } else if(tecla == KEY_DOWN) {
            seleccion = (seleccion + 1) % 4;
        } else if(tecla == '\n') { // ENTER
            if(seleccion == 0) iniciar_juego();
            else if(seleccion == 1) mostrar_instrucciones();
            else if(seleccion == 2) mostrar_puntajes();
            else if(seleccion == 3) break;
        }
    }

    endwin();
    return 0;
}