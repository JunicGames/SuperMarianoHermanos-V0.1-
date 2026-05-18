/*
 * mario.cpp - Super Mario Bros. ASCII - Fase 2: Entorno Gráfico (Migrado a C++)
 * Universidad del Valle de Guatemala
 * Programación de Microprocesadores - Primer Ciclo 2026
 *
 * Compilar con: g++ mario.cpp -o mario -lncurses
 * Requiere: ncurses (soporta hilos estándar de C++11)
 */

#include <ncurses/ncurses.h>
#include <string>
#include <vector>
#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <ctime>
#include <thread>
#include <mutex>
#include <cmath>
#include <algorithm>

/* ===================== CONSTANTES ===================== */
const int BOARD_WIDTH   = 80;
const int BOARD_HEIGHT  = 24;
const int GROUND_ROW    = 23;
const int MAX_ENEMIES    = 5;
const int MAX_COINS     = 10;
const int MAX_SCORES     = 5;
const int MAX_PROJECTILES = 2;

/* ===================== ESTRUCTURAS Y CLASES ===================== */
struct HighScore {
    std::string name;
    int score;
};

class Enemy {
public:
    int x, y;
    bool alive;
    int dir;       // -1 izquierda, 1 derecha
    int type;      // 0=Goomba, 1=Koopa, 2=Shell

    Enemy(int nx, int ny, int ntype, int ndir) 
        : x(nx), y(ny), alive(true), dir(ndir), type(ntype) {}
};

class Coin {
public:
    int x, y;
    bool active;

    Coin(int nx, int ny) : x(nx), y(ny), active(true) {}
};

class Projectile {
public:
    int x, y;
    bool active;
    int dir;

    Projectile() : x(0), y(0), active(false), dir(1) {}
};

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

/* ===================== CLASE PRINCIPAL DEL JUEGO ===================== */
class Game {
private:
    MarioPlayer mario;
    std::vector<Enemy> enemies;
    std::vector<Coin> coins;
    std::vector<Projectile> projectiles;
    std::vector<HighScore> high_scores;
    
    int game_mode;     // 1=lento, 2=rápido
    bool game_running;
    bool game_paused;
    int level_time;

    std::mutex board_mutex;

    // Elementos del mapa fijaos
    const std::vector<std::vector<int>> platforms = {
        {23,  0, 79},   // suelo principal
        {18, 10, 20},
        {15, 25, 38},
        {12, 45, 58},
        {18, 60, 72},
        {10, 30, 40},
    };

    const std::vector<std::vector<int>> qblocks = {
        {14, 15}, {14, 30}, {14, 47}, {11, 35}
    };
    std::vector<int> qblock_state;

    const std::vector<std::vector<int>> pipes = {
        {20, 5}, {20, 68}
    };

    /* ===================== UTILIDADES DE DIBUJO ===================== */
    void draw_centered(int row, const std::string &str) {
        int col = (BOARD_WIDTH - str.length()) / 2;
        if (col < 0) col = 0;
        mvprintw(row, col, "%s", str.c_str());
    }

    void draw_box(int y, int x, int h, int w) {
        mvaddch(y,     x,     ACS_ULCORNER);
        mvaddch(y,     x+w-1, ACS_URCORNER);
        mvaddch(y+h-1, x,     ACS_LLCORNER);
        mvaddch(y+h-1, x+w-1, ACS_LRCORNER);
        for (int i = 1; i < w-1; i++) {
            mvaddch(y,     x+i, ACS_HLINE);
            mvaddch(y+h-1, x+i, ACS_HLINE);
        }
        for (int i = 1; i < h-1; i++) {
            mvaddch(y+i, x,     ACS_VLINE);
            mvaddch(y+i, x+w-1, ACS_VLINE);
        }
    }

public:
    Game() : game_mode(1), game_running(false), game_paused(false), level_time(300) {
        // Inicializar puntajes por defecto
        high_scores.push_back({"LUIGI", 12500});
        high_scores.push_back({"PEACH", 8200});
        high_scores.push_back({"TOAD", 5500});
        projectiles.resize(MAX_PROJECTILES);
        qblock_state.resize(qblocks.size(), 0);
    }

    /* ===================== PANTALLAS ===================== */
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
            draw_centered(2, "  ____  _   _ ____  _____ ____  ");
            draw_centered(3, " / ___|| | | |  _ \\| ____|  _ \\ ");
            draw_centered(4, " \\___ \\| | | | |_) |  _| | |_) |");
            draw_centered(5, "  ___) | |_| |  __/| |___|  _ < ");
            draw_centered(6, " |____/ \\___/|_|   |_____|_| \\_\\");
            attroff(COLOR_PAIR(3) | A_BOLD);

            attron(COLOR_PAIR(2) | A_BOLD);
            draw_centered(8, " __  __    _    ____  ___    _    _   _  ___ ");
            draw_centered(9, "|  \\/  |  / \\  |  _ \\|_ _|  / \\  | \\ | |/ _ \\");
            draw_centered(10,"| |\\/| | / _ \\ | |_) || |  / _ \\ |  \\| | | | |");
            draw_centered(11,"| |  | |/ ___ \\|  _ < | | / ___ \\| |\\  | |_| |");
            draw_centered(12,"|_|  |_/_/   \\_\\_| \\_\\___/_/   \\_\\_| \\_|\\___/");
            attroff(COLOR_PAIR(2) | A_BOLD);

            attron(COLOR_PAIR(1));
            draw_centered(13, "  ( ^>)  [?]  (oo)  ||==||  ======  o  [#]  ~o~  |P ");
            attroff(COLOR_PAIR(1));

            for (int i = 0; i < n; i++) {
                if (i == sel) {
                    attron(COLOR_PAIR(4) | A_REVERSE | A_BOLD);
                } else {
                    attron(COLOR_PAIR(1));
                }
                draw_centered(20 + i, opts[i]);
                attroff(COLOR_PAIR(4) | A_REVERSE | A_BOLD);
                attroff(COLOR_PAIR(1));
            }

            attron(COLOR_PAIR(5));
            draw_centered(21, "Usa [^] [v] para navegar  |  ENTER para confirmar");
            draw_centered(22, "Universidad del Valle de Guatemala - Microprocesadores 2026");
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
            draw_box(3, 15, 18, 50);

            attron(COLOR_PAIR(3) | A_BOLD);
            draw_centered(5, "=== SELECCIONAR MODO DE JUEGO ===");
            attroff(COLOR_PAIR(3) | A_BOLD);

            mvprintw(8, 18, "Selecciona la dificultad:");

            if (sel == 0) attron(COLOR_PAIR(4) | A_REVERSE | A_BOLD);
            else          attron(COLOR_PAIR(2));
            mvprintw(11, 20, "  MODO 1 - LENTO  ");
            attroff(COLOR_PAIR(4) | A_REVERSE | A_BOLD);
            attroff(COLOR_PAIR(2));

            attron(COLOR_PAIR(1));
            mvprintw(12, 20, "  Velocidad moderada, mas tiempo");
            mvprintw(13, 20, "  de reaccion. Ideal para aprender.");
            attroff(COLOR_PAIR(1));

            if (sel == 1) attron(COLOR_PAIR(4) | A_REVERSE | A_BOLD);
            else          attron(COLOR_PAIR(3));
            mvprintw(16, 20, "  MODO 2 - RAPIDO  ");
            attroff(COLOR_PAIR(4) | A_REVERSE | A_BOLD);
            attroff(COLOR_PAIR(3));

            attron(COLOR_PAIR(1));
            mvprintw(17, 20, "  Enemigos rapidos, mas proyectiles.");
            mvprintw(18, 20, "  Puntos x1.5 al finalizar nivel.");
            attroff(COLOR_PAIR(1));

            attron(COLOR_PAIR(5));
            mvprintw(21, 18, "[^][v] Navegar  |  ENTER Confirmar  |  ESC Volver");
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
        draw_box(0, 0, 24, 80);

        attron(COLOR_PAIR(3) | A_BOLD);
        draw_centered(1, "INSTRUCCIONES DEL JUEGO");
        attroff(COLOR_PAIR(3) | A_BOLD);

        attron(COLOR_PAIR(2) | A_BOLD);
        mvprintw(3, 3, "OBJETIVO:");
        attroff(COLOR_PAIR(2) | A_BOLD);
        attron(COLOR_PAIR(1));
        mvprintw(4, 3, "Guia a Mario de izquierda a derecha, elimina enemigos,");
        mvprintw(5, 3, "recoge monedas y llega a la bandera |P para completar el nivel.");
        attroff(COLOR_PAIR(1));

        attron(COLOR_PAIR(2) | A_BOLD);
        mvprintw(7, 3, "CONTROLES:");
        attroff(COLOR_PAIR(2) | A_BOLD);
        attron(COLOR_PAIR(1));
        mvprintw( 8, 5, "<- / ->      Mover a Mario izquierda / derecha");
        mvprintw( 9, 5, "^ o ESPACIO  Saltar (trayectoria parabolica)");
        mvprintw(10, 5, "Z o A        Lanzar proyectil de fuego (requiere flor)");
        mvprintw(11, 5, "P            Pausar / Reanudar el juego");
        mvprintw(12, 5, "R            Reiniciar partida");
        mvprintw(13, 5, "Q / ESC      Salir del programa");
        attroff(COLOR_PAIR(1));

        attron(COLOR_PAIR(2) | A_BOLD);
        mvprintw(15, 3, "ELEMENTOS VISUALES:");
        attroff(COLOR_PAIR(2) | A_BOLD);
        attron(COLOR_PAIR(1));
        mvprintw(16, 5, "( ^>)  Mario normal     ( ^>) ( )  Super Mario");
        mvprintw(17, 5, "(oo)/mm\\  Goomba        (^^)[==]   Koopa Troopa");
        mvprintw(18, 5, "[?]  Bloque especial    [#]  Bloque ladrillo");
        mvprintw(19, 5, "o    Moneda             ||==||  Tuberia");
        mvprintw(20, 5, "~o~  Proyectil fuego    |P   Meta/Bandera");
        attroff(COLOR_PAIR(1));

        attron(COLOR_PAIR(3));
        mvprintw(22, 3, "(*) Recoge hongos para crecer. Recoge la flor para lanzar fuego.");
        attroff(COLOR_PAIR(3));

        attron(COLOR_PAIR(5));
        draw_centered(23, "Presiona cualquier tecla para volver...");
        attroff(COLOR_PAIR(5));

        refresh();
        nodelay(stdscr, FALSE);
        getch();
        nodelay(stdscr, TRUE);
    }

    void scores_screen() {
        clear();
        draw_box(2, 15, 20, 50);

        attron(COLOR_PAIR(3) | A_BOLD);
        draw_centered(4, "*** MEJORES PUNTAJES ***");
        attroff(COLOR_PAIR(3) | A_BOLD);

        mvprintw(6, 20, "  #    Jugador          Puntaje");
        mvprintw(7, 20, "  --------------------------------");

        if (high_scores.empty()) {
            attron(COLOR_PAIR(5));
            draw_centered(12, "  No hay puntajes registrados aun.");
            attroff(COLOR_PAIR(5));
        } else {
            for (size_t i = 0; i < high_scores.size(); i++) {
                if (i == 0) attron(COLOR_PAIR(3) | A_BOLD);
                else        attron(COLOR_PAIR(1));
                mvprintw(8 + i, 20, "  %zu    %-15s  %7d",
                         i+1, high_scores[i].name.c_str(), high_scores[i].score);
                attroff(COLOR_PAIR(3) | A_BOLD);
                attroff(COLOR_PAIR(1));
            }
        }

        attron(COLOR_PAIR(5));
        draw_centered(20, "Presiona cualquier tecla para volver...");
        attroff(COLOR_PAIR(5));

        refresh();
        nodelay(stdscr, FALSE);
        getch();
        nodelay(stdscr, TRUE);
    }

    /* ===================== LÓGICA DE INICIALIZACIÓN Y HUD ===================== */
    void init_game() {
        mario.reset();
        level_time = 300;

        // Inicializar Enemigos
        enemies.clear();
        int ex[] = {20, 35, 50, 65, 28};
        for (int i = 0; i < MAX_ENEMIES; i++) {
            int type = (i < 3) ? 0 : 1; // 0=goomba, 1=koopa
            int dir = (i % 2 == 0) ? -1 : 1;
            enemies.push_back(Enemy(ex[i], GROUND_ROW - 1, type, dir));
        }

        // Inicializar Monedas
        coins.clear();
        int cx[] = {8,12,18,24,32,40,48,55,63,70};
        for (int i = 0; i < MAX_COINS; i++) {
            coins.push_back(Coin(cx[i], GROUND_ROW - 2));
        }

        // Resetear Proyectiles
        for (auto &p : projectiles) {
            p.active = false;
        }

        // Resetear bloques
        std::fill(qblock_state.begin(), qblock_state.end(), 0);
    }

    void draw_hud() {
        attron(COLOR_PAIR(6) | A_BOLD);
        int hi_score = high_scores.empty() ? 0 : high_scores[0].score;
        mvprintw(0, 0,
            " MARIO  SCORE:%-7d  LIVES:%d  COINS:%-3d  TIME:%-4d  MODO:%d  HI:%d ",
            mario.score, mario.lives, mario.coins, level_time,
            game_mode, hi_score);
        attroff(COLOR_PAIR(6) | A_BOLD);
    }

    void draw_board() {
        // Fondo / cielo
        attron(COLOR_PAIR(7));
        for (int r = 1; r < BOARD_HEIGHT; r++)
            for (int c = 0; c < BOARD_WIDTH; c++)
                mvaddch(r, c, ' ');
        attroff(COLOR_PAIR(7));

        // Plataformas
        attron(COLOR_PAIR(2) | A_BOLD);
        for (const auto &plat : platforms) {
            int row = plat[0];
            int c0  = plat[1];
            int c1  = plat[2];
            for (int c = c0; c <= c1; c++)
                mvprintw(row, c, "=");
        }
        attroff(COLOR_PAIR(2) | A_BOLD);

        // Bloques [?]
        for (size_t i = 0; i < qblocks.size(); i++) {
            if (qblock_state[i] == 0) {
                attron(COLOR_PAIR(3) | A_BOLD);
                mvprintw(qblocks[i][0], qblocks[i][1], "[?]");
                attroff(COLOR_PAIR(3) | A_BOLD);
            } else {
                attron(COLOR_PAIR(5));
                mvprintw(qblocks[i][0], qblocks[i][1], "[*]");
                attroff(COLOR_PAIR(5));
            }
        }

        // Tubos
        attron(COLOR_PAIR(2));
        for (const auto &pipe : pipes) {
            int r = pipe[0];
            int c = pipe[1];
            mvprintw(r - 1, c, "||==||");
            mvprintw(r,     c, "||  ||");
            mvprintw(r + 1, c, "||  ||");
        }
        attroff(COLOR_PAIR(2));

        // Monedas
        attron(COLOR_PAIR(3));
        for (const auto &coin : coins) {
            if (coin.active)
                mvprintw(coin.y, coin.x, "o");
        }
        attroff(COLOR_PAIR(3));

        // Bandera meta
        attron(COLOR_PAIR(3) | A_BOLD);
        for(int i = 5; i >= 1; i--) {
            mvprintw(GROUND_ROW - i, 76, (i == 5) ? "|P" : "| ");
        }
        attroff(COLOR_PAIR(3) | A_BOLD);

        // Enemigos
        for (const auto &enemy : enemies) {
            if (!enemy.alive) continue;
            attron(COLOR_PAIR(1) | A_BOLD);
            if (enemy.type == 0) {
                mvprintw(enemy.y - 1, enemy.x, "(oo)");
                mvprintw(enemy.y,     enemy.x, "/mm\\");
            } else if (enemy.type == 1) {
                mvprintw(enemy.y - 1, enemy.x, "(^^)");
                mvprintw(enemy.y,     enemy.x, "[==]");
            } else {
                mvprintw(enemy.y, enemy.x, "[==]");
            }
            attroff(COLOR_PAIR(1) | A_BOLD);
        }

        // Proyectiles
        attron(COLOR_PAIR(4) | A_BOLD);
        for (const auto &proj : projectiles) {
            if (proj.active)
                mvprintw(proj.y, proj.x, "~o~");
        }
        attroff(COLOR_PAIR(4) | A_BOLD);

        // Mario
        attron(COLOR_PAIR(4) | A_BOLD);
        if (mario.state == 0) {
            if (mario.dir == 1)  mvprintw(mario.y, mario.x, "( ^>)");
            else                 mvprintw(mario.y, mario.x, "(<^ )");
        } else { 
            if (mario.dir == 1) {
                mvprintw(mario.y - 1, mario.x, "( ^>)");
                mvprintw(mario.y,     mario.x, "( || )");
            } else {
                mvprintw(mario.y - 1, mario.x, "(<^ )");
                mvprintw(mario.y,     mario.x, "( || )");
            }
        }
        attroff(COLOR_PAIR(4) | A_BOLD);
    }

    /* ===================== FUNCIONES DE HILOS (LOOPS) ===================== */
    void timer_loop() {
        while (game_running) {
            usleep(1000000);
            std::lock_guard<std::mutex> lock(board_mutex);
            if (!game_paused && level_time > 0)
                level_time--;
        }
    }

    void enemies_loop() {
        int speed_us = (game_mode == 1) ? 300000 : 150000;
        while (game_running) {
            usleep(speed_us);
            std::lock_guard<std::mutex> lock(board_mutex);
            if (!game_paused) {
                for (auto &enemy : enemies) {
                    if (!enemy.alive) continue;
                    enemy.x += enemy.dir;
                    if (enemy.x <= 1 || enemy.x >= BOARD_WIDTH - 5)
                        enemy.dir *= -1;

                    bool on_p = false;
                    for (const auto &plat : platforms) {
                        if (enemy.y == plat[0] - 1 && enemy.x >= plat[1] && enemy.x <= plat[2]) {
                            on_p = true; 
                            break;
                        }
                    }
                    if (!on_p && enemy.y < GROUND_ROW - 1)
                        enemy.y++;
                }
            }
        }
    }

    void projectiles_loop() {
        while (game_running) {
            usleep(80000);
            std::lock_guard<std::mutex> lock(board_mutex);
            if (!game_paused) {
                for (auto &proj : projectiles) {
                    if (!proj.active) continue;
                    proj.x += 2 * proj.dir;
                    if (proj.x < 1 || proj.x > BOARD_WIDTH - 4)
                        proj.active = false;

                    for (auto &enemy : enemies) {
                        if (!enemy.alive) continue;
                        if (std::abs(proj.x - enemy.x) < 4 && std::abs(proj.y - enemy.y) < 2) {
                            enemy.alive = false;
                            proj.active = false;
                            mario.score += 150;
                            break;
                        }
                    }
                }
            }
        }
    }

    /* ===================== LÓGICA DE MARIO ===================== */
    void update_mario(int key) {
        if (key == KEY_LEFT && mario.x > 1) {
            mario.x--;
            mario.dir = -1;
        }
        if (key == KEY_RIGHT && mario.x < BOARD_WIDTH - 6) {
            mario.x++;
            mario.dir = 1;
        }
        if ((key == KEY_UP || key == ' ') && mario.on_ground) {
            mario.vy = -4;
            mario.on_ground = false;
        }
        if ((key == 'z' || key == 'Z' || key == 'a' || key == 'A') && mario.has_fire) {
            for (auto &proj : projectiles) {
                if (!proj.active) {
                    proj.x = mario.x + (mario.dir == 1 ? 5 : -3);
                    proj.y = mario.y;
                    proj.dir = mario.dir;
                    proj.active = true;
                    break;
                }
            }
        }

        mario.y += mario.vy;
        if (mario.vy < 3) mario.vy++;

        mario.on_ground = false;
        if (mario.y >= GROUND_ROW - 1) {
            mario.y = GROUND_ROW - 1;
            mario.vy = 0;
            mario.on_ground = true;
        }
        for (const auto &plat : platforms) {
            if (mario.y == plat[0] - 1 &&
                mario.x + 4 >= plat[1] &&
                mario.x <= plat[2] && mario.vy >= 0) {
                mario.y = plat[0] - 1;
                mario.vy = 0;
                mario.on_ground = true;
            }
        }
        if (mario.y < 1) mario.y = 1;

        // Recolectar monedas
        for (auto &coin : coins) {
            if (!coin.active) continue;
            if (std::abs(mario.x - coin.x) < 5 && std::abs(mario.y - coin.y) < 2) {
                coin.active = false;
                mario.score += 100;
                mario.coins++;
                if (mario.coins >= 100) { mario.lives++; mario.coins = 0; }
            }
        }

        // Colisión con enemigos
        for (auto &enemy : enemies) {
            if (!enemy.alive) continue;
            int dx = std::abs(mario.x - enemy.x);
            int dy = mario.y - enemy.y;
            if (dx < 4) {
                if (dy == -1 && mario.vy > 0) { // Pisó enemigo
                    enemy.alive = false;
                    mario.score += 100;
                    mario.vy = -2;
                } else if (std::abs(dy) < 2) { // Golpe lateral
                    if (!mario.invincible) {
                        if (mario.state > 0) {
                            mario.state = 0;
                            mario.has_fire = false;
                        } else {
                            mario.lives--;
                            mario.x = 5; mario.y = GROUND_ROW - 1;
                            mario.vy = 0; mario.on_ground = true;
                        }
                    }
                }
            }
        }

        // Golpear bloque [?] desde abajo
        for (size_t i = 0; i < qblocks.size(); i++) {
            if (qblock_state[i]) continue;
            if (std::abs(mario.x - qblocks[i][1]) < 4 &&
                mario.y - 1 == qblocks[i][0] && mario.vy < 0) {
                qblock_state[i] = 1;
                mario.score += 50;
                mario.state = 1; // Crecer a Super Mario
            }
        }
    }

    void draw_pause() {
        attron(COLOR_PAIR(6) | A_BOLD | A_REVERSE);
        draw_centered(10, "                                    ");
        draw_centered(11, "          === JUEGO PAUSADO ===     ");
        draw_centered(12, "      Presiona P para continuar     ");
        draw_centered(13, "                                    ");
        attroff(COLOR_PAIR(6) | A_BOLD | A_REVERSE);
    }

    void result_screen(bool won) {
        nodelay(stdscr, FALSE);
        clear();
        draw_box(4, 15, 16, 50);

        attron(COLOR_PAIR(won ? 3 : 1) | A_BOLD);
        if (won) draw_centered(6, "*** NIVEL COMPLETADO! ***");
        else     draw_centered(6, "*** GAME OVER ***");
        attroff(COLOR_PAIR(won ? 3 : 1) | A_BOLD);

        attron(COLOR_PAIR(2));
        mvprintw( 9, 20, "  Puntuacion final : %d", mario.score);
        mvprintw(10, 20, "  Monedas recogidas: %d", mario.coins);
        mvprintw(11, 20, "  Vidas restantes  : %d", mario.lives);
        mvprintw(12, 20, "  Tiempo restante  : %d seg", level_time);
        if (won)
            mvprintw(13, 20, "  Bonus tiempo     : +%d pts", level_time * 50);
        attroff(COLOR_PAIR(2));

        // Registrar puntaje al estilo C++ (vectores y algoritmos)
        int final_score = mario.score + (won ? level_time * 50 : 0);
        if (high_scores.size() < MAX_SCORES || final_score > high_scores.back().score) {
            std::string name = "MARIO-" + std::to_string(high_scores.size() + 1);
            high_scores.push_back({name, final_score});
            
            // Ordenar de mayor a menor utilizando lambdas de C++
            std::sort(high_scores.begin(), high_scores.end(), [](const HighScore &a, const HighScore &b) {
                return a.score > b.score;
            });

            if (high_scores.size() > MAX_SCORES) {
                high_scores.pop_back();
            }
        }

        attron(COLOR_PAIR(5));
        mvprintw(16, 20, "  [R] Reiniciar   [Q/ESC] Salir");
        attroff(COLOR_PAIR(5));

        refresh();
        while (true) {
            int ch = getch();
            if (ch == 'r' || ch == 'R') break;
            if (ch == 'q' || ch == 'Q' || ch == 27) { game_running = false; break; }
        }
        nodelay(stdscr, TRUE);
    }

    void game_loop() {
        init_game();
        game_running = true;
        game_paused  = false;

        // Despliegue de Hilos utilizando std::thread de C++11
        std::thread t_timer(&Game::timer_loop, this);
        std::thread t_enemies(&Game::enemies_loop, this);
        std::thread t_proj(&Game::projectiles_loop, this);

        bool won = false;
        while (game_running) {
            int ch = getch();
            if (ch == 'q' || ch == 'Q' || ch == 27) { game_running = false; break; }
            if (ch == 'p' || ch == 'P') { game_paused = !game_paused; }

            if (!game_paused) {
                std::lock_guard<std::mutex> lock(board_mutex);
                update_mario(ch);
            }

            if (mario.lives <= 0 || level_time <= 0) {
                game_running = false; break;
            }
            if (mario.x >= 75) {
                won = true; game_running = false; break;
            }

            erase();
            draw_hud();
            {
                std::lock_guard<std::mutex> lock(board_mutex);
                draw_board();
            }
            if (game_paused) draw_pause();
            refresh();

            usleep(40000); // ~25 FPS
        }

        // Sincronización y cierre correcto de hilos
        if (t_timer.joinable()) t_timer.join();
        if (t_enemies.joinable()) t_enemies.join();
        if (t_proj.joinable()) t_proj.join();

        result_screen(won);
    }

    void set_mode(int mode) { game_mode = mode; }
};

/* ===================== FUNCIÓN PRINCIPAL MAIN ===================== */
int main() {
    // Iniciar ncurses
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
    nodelay(stdscr, TRUE);

    if (has_colors()) {
        start_color();
        use_default_colors();
        init_pair(1, COLOR_WHITE,   -1); // elementos normales
        init_pair(2, COLOR_GREEN,   -1); // plataformas
        init_pair(3, COLOR_YELLOW,  -1); // bloques / victoria
        init_pair(4, COLOR_RED,     -1); // mario / proyectiles
        init_pair(5, COLOR_CYAN,    -1); // info / pie de página
        init_pair(6, COLOR_BLACK,   COLOR_WHITE); // HUD
        init_pair(7, COLOR_BLUE,    -1); // cielo
    }

    std::srand(static_cast<unsigned>(std::time(nullptr)));

    // Instancia principal del juego
    Game super_mario;

    while (true) {
        int opt = super_mario.menu_screen();
        if (opt == 0) {
            int mode = super_mario.mode_select_screen();
            if (mode > 0) {
                super_mario.set_mode(mode);
                super_mario.game_loop();
            }
        } else if (opt == 1) {
            super_mario.instructions_screen();
        } else if (opt == 2) {
            super_mario.scores_screen();
        } else {
            break;
        }
    }

    endwin();
    return 0;
}