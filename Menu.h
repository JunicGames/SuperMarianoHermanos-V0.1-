#pragma once
#include <vector>
#include <string>
#include <ncurses/ncurses.h>
#include "Renderer.h"


class Menu {
private:
    Renderer renderer;
    ScoreManager scores;

public:

    int menu_screen() {
        int sel = 0;
        std::vector<std::string> opts = {
            "  [1] Iniciar Partida  ",
            "  [2] Instrucciones    ",
            "  [3] Puntajes         ",
            "  [4] Salir            "
        };
        int n = opts.size();

        while (true) {
            erase();
            attron(COLOR_PAIR(3) | A_BOLD);
            renderer.draw_centered(2, "  ____  _   _ ____  _____ ____  ");
            renderer.draw_centered(3, " / ___|| | | |  _ \\| ____|  _ \\ ");
            renderer.draw_centered(4, " \\___ \\| | | | |_) |  _| | |_) |");
            renderer.draw_centered(5, "  ___) | |_| |  __/| |___|  _ < ");
            renderer.draw_centered(6, " |____/ \\___/|_|   |_____|_| \\_\\");
            attroff(COLOR_PAIR(3) | A_BOLD);

            attron(COLOR_PAIR(2) | A_BOLD);
            renderer.draw_centered(8, " __  __    _    ____  ___    _    _   _  ___ ");
            renderer.draw_centered(9, "|  \\/  |  / \\  |  _ \\|_ _|  / \\  | \\ | |/ _ \\");
            renderer.draw_centered(10,"| |\\/| | / _ \\ | |_) || |  / _ \\ |  \\| | | | |");
            renderer.draw_centered(11,"| |  | |/ ___ \\|  _ < | | / ___ \\| |\\  | |_| |");
            renderer.draw_centered(12,"|_|  |_/_/   \\_\\_| \\_\\___/_/   \\_\\_| \\_|\\___/");
            attroff(COLOR_PAIR(2) | A_BOLD);

            attron(COLOR_PAIR(1));
            renderer.draw_centered(13, "  ( ^>)  [?]  (oo)  ||==||  ======  o  [#]  ~o~  |P ");
            attroff(COLOR_PAIR(1));

            for (int i = 0; i < n; i++) {
                if (i == sel) {
                    attron(COLOR_PAIR(4) | A_REVERSE | A_BOLD);
                } else {
                    attron(COLOR_PAIR(1));
                }
                renderer.draw_centered(20 + i, opts[i]);
                attroff(COLOR_PAIR(4) | A_REVERSE | A_BOLD);
                attroff(COLOR_PAIR(1));
            }

            attron(COLOR_PAIR(5));
            renderer.draw_centered(21, "Usa [^] [v] para navegar  |  ENTER para confirmar");
            renderer.draw_centered(22, "Universidad del Valle de Guatemala - Microprocesadores 2026");
            attroff(COLOR_PAIR(5));
            refresh();

            int ch = getch();
            if (ch == KEY_UP)    sel = (sel - 1 + n) % n;
            if (ch == KEY_DOWN)  sel = (sel + 1) % n;
            if (ch == '\n' || ch == '\r' || ch == ' ') return sel;
            if (ch == '1') return 0;
            if (ch == '2') return 1;
            if (ch == '3') return 2;
            if (ch == '4') return 3;
            if (ch == 'q' || ch == 'Q' || ch == 27) return 3;
        }
    }

    int mode_select_screen() {
        int sel = 0;
        while (true) {
            erase();
            renderer.draw_box(3, 15, 18, 50);

            attron(COLOR_PAIR(3) | A_BOLD);
            renderer.draw_centered(5, "=== SELECCIONAR MODO DE JUEGO ===");
            attroff(COLOR_PAIR(3) | A_BOLD);

            renderer.draw_centered(8, "Selecciona la dificultad:");

            if (sel == 0) attron(COLOR_PAIR(4) | A_REVERSE | A_BOLD);
            else          attron(COLOR_PAIR(2));
            renderer.draw_centered(11, "  MODO 1 - LENTO  ");
            attroff(COLOR_PAIR(4) | A_REVERSE | A_BOLD);
            attroff(COLOR_PAIR(2));

            attron(COLOR_PAIR(1));
            renderer.draw_centered(12, "  Velocidad moderada, mas tiempo");
            renderer.draw_centered(13, "  de reaccion. Ideal para aprender.");
            attroff(COLOR_PAIR(1));

            if (sel == 1) attron(COLOR_PAIR(4) | A_REVERSE | A_BOLD);
            else          attron(COLOR_PAIR(3));
            renderer.draw_centered(16, "  MODO 2 - RAPIDO  ");
            attroff(COLOR_PAIR(4) | A_REVERSE | A_BOLD);
            attroff(COLOR_PAIR(3));

            attron(COLOR_PAIR(1));
            renderer.draw_centered(17, "  Enemigos rapidos, mas proyectiles.");
            renderer.draw_centered(18, "  Puntos x1.5 al finalizar nivel.");
            attroff(COLOR_PAIR(1));

            attron(COLOR_PAIR(5));
            renderer.draw_centered(21, "[^][v] Navegar  |  ENTER Confirmar  |  ESC Volver");
            attroff(COLOR_PAIR(5));

            refresh();
            int ch = getch();
            if (ch == KEY_UP || ch == KEY_DOWN) sel ^= 1;
            if (ch == '\n' || ch == '\r' || ch == ' ') return sel + 1;
            if (ch == 27) return -1;
        }
    }

    void instructions_screen() {
        clear();
        renderer.draw_box(0, 0, 24, 80);

        attron(COLOR_PAIR(3) | A_BOLD);
        renderer.draw_centered(1, "INSTRUCCIONES DEL JUEGO");
        attroff(COLOR_PAIR(3) | A_BOLD);

        attron(COLOR_PAIR(2) | A_BOLD);
        renderer.draw_centered(3, "OBJETIVO:");
        attroff(COLOR_PAIR(2) | A_BOLD);
        attron(COLOR_PAIR(1));
        renderer.draw_centered(4, "Guia a Mario de izquierda a derecha, elimina enemigos,");
        renderer.draw_centered(5, "recoge monedas y llega a la bandera |P para completar el nivel.");
        attroff(COLOR_PAIR(1));

        attron(COLOR_PAIR(2) | A_BOLD);
        renderer.draw_centered(7, "CONTROLES:");
        attroff(COLOR_PAIR(2) | A_BOLD);
        attron(COLOR_PAIR(1));
        renderer.draw_centered(8, "<- / ->      Mover a Mario izquierda / derecha");
        renderer.draw_centered(9, "^ o ESPACIO  Saltar (trayectoria parabolica)");
        renderer.draw_centered(10, "Z o A        Lanzar proyectil de fuego (requiere flor)");
        renderer.draw_centered(11, "P            Pausar / Reanudar el juego");
        renderer.draw_centered(12, "R            Reiniciar partida");
        renderer.draw_centered(13, "Q / ESC      Salir del programa");
        attroff(COLOR_PAIR(1));

        attron(COLOR_PAIR(2) | A_BOLD);
        renderer.draw_centered(15, "ELEMENTOS VISUALES:");
        attroff(COLOR_PAIR(2) | A_BOLD);
        attron(COLOR_PAIR(1));
        renderer.draw_centered(16, "( ^>)  Mario normal     ( ^>) ( )  Super Mario");
        renderer.draw_centered(17, "(oo)/mm\\  Goomba        (^^)[==]   Koopa Troopa");
        renderer.draw_centered(18, "[?]  Bloque especial    [#]  Bloque ladrillo");
        renderer.draw_centered(19, "o    Moneda             ||==||  Tuberia");
        renderer.draw_centered(20, "~o~  Proyectil fuego    |P   Meta/Bandera");
        attroff(COLOR_PAIR(1));

        attron(COLOR_PAIR(3));
        renderer.draw_centered(22, "(*) Recoge hongos para crecer. Recoge la flor para lanzar fuego.");
        attroff(COLOR_PAIR(3));

        attron(COLOR_PAIR(5));
        renderer.draw_centered(23, "Presiona cualquier tecla para volver...");
        attroff(COLOR_PAIR(5));

        refresh();
        nodelay(stdscr, FALSE);
        getch();
        nodelay(stdscr, TRUE);
    }

    void scores_screen() {
        clear();
        renderer.draw_box(2, 15, 20, 50);

        attron(COLOR_PAIR(3) | A_BOLD);
        renderer.draw_centered(4, "*** MEJORES PUNTAJES ***");
        attroff(COLOR_PAIR(3) | A_BOLD);

        renderer.draw_centered(6, "  #    Jugador          Puntaje");
        renderer.draw_centered(7, "  --------------------------------");

        if (scores.get_scores().empty()) {
            attron(COLOR_PAIR(5));
            renderer.draw_centered(12, "  No hay puntajes registrados aun.");
            attroff(COLOR_PAIR(5));
        } else {
            for (size_t i = 0; i < scores.get_scores().size(); i++) {
                if (i == 0) attron(COLOR_PAIR(3) | A_BOLD);
                else        attron(COLOR_PAIR(1));
                std::string texto = std::to_string(i + 1) + "    " + scores.get_scores()[i].name + "    " + std::to_string(scores.get_scores()[i].score);
                renderer.draw_centered(8 + i, texto);
                attroff(COLOR_PAIR(3) | A_BOLD);
                attroff(COLOR_PAIR(1));
            }
        }

        attron(COLOR_PAIR(5));
        renderer.draw_centered(20, "Presiona cualquier tecla para volver...");
        attroff(COLOR_PAIR(5));

        refresh();
        nodelay(stdscr, FALSE);
        getch();
        nodelay(stdscr, TRUE);
    }

};