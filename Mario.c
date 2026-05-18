/*
 * mario.c - Super Mario Bros. ASCII - Fase 2: Entorno Gráfico
 * Universidad del Valle de Guatemala
 * Programación de Microprocesadores - Primer Ciclo 2026
 *
 * Compilar con: gcc mario.c -o mario -lncurses -lpthread
 * Requiere: ncurses, pthread
 */

#include <ncurses/ncurses.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>

/* ===================== CONSTANTES ===================== */
#define BOARD_WIDTH   80
#define BOARD_HEIGHT  24
#define HUD_ROW        0
#define GROUND_ROW    23
#define MAX_ENEMIES    5
#define MAX_COINS     10
#define MAX_SCORES     5
#define MAX_PROJECTILES 2

/* ===================== ESTRUCTURAS ===================== */
typedef struct {
    int x, y;
    int alive;
    int dir;       /* -1 izquierda, 1 derecha */
    int type;      /* 0=Goomba, 1=Koopa, 2=Shell */
} Enemy;

typedef struct {
    int x, y;
    int active;
} Coin;

typedef struct {
    int x, y;
    int active;
    int dir;
} Projectile;

typedef struct {
    int x, y;
    int vy;        /* velocidad vertical */
    int on_ground;
    int state;     /* 0=normal, 1=super, 2=fire */
    int invincible;
    int lives;
    int score;
    int coins;
    int has_fire;
    int dir;       /* 1=derecha, -1=izquierda */
} Mario;

typedef struct {
    char name[16];
    int  score;
} HighScore;

/* ===================== GLOBALES ===================== */
Mario       mario;
Enemy       enemies[MAX_ENEMIES];
Coin        coins[MAX_COINS];
Projectile  projectiles[MAX_PROJECTILES];
HighScore   high_scores[MAX_SCORES];
int         game_mode     = 1;   /* 1=lento, 2=rápido */
int         game_running  = 0;
int         game_paused   = 0;
int         level_time    = 300;
int         num_scores    = 0;

pthread_mutex_t board_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  pause_cond  = PTHREAD_COND_INITIALIZER;

/* Plataformas: fila, col_inicio, col_fin */
int platforms[][3] = {
    {23,  0, 79},   /* suelo principal */
    {18, 10, 20},
    {15, 25, 38},
    {12, 45, 58},
    {18, 60, 72},
    {10, 30, 40},
};
int num_platforms = 6;

/* Bloques '?' */
int qblocks[][2] = {{14,15},{14,30},{14,47},{11,35}};
int qblock_state[4] = {0,0,0,0}; /* 0=activo, 1=golpeado */
int num_qblocks = 4;

/* Tubos */
int pipes[][2] = {{20, 5},{20, 68}};
int num_pipes = 2;

/* ===================== UTILIDADES ===================== */
void draw_centered(int row, const char *str) {
    int col = (BOARD_WIDTH - strlen(str)) / 2;
    if (col < 0) col = 0;
    mvprintw(row, col, "%s", str);
}

void draw_box(int y, int x, int h, int w) {
    /* esquinas */
    mvaddch(y,     x,     ACS_ULCORNER);
    mvaddch(y,     x+w-1, ACS_URCORNER);
    mvaddch(y+h-1, x,     ACS_LLCORNER);
    mvaddch(y+h-1, x+w-1, ACS_LRCORNER);
    /* bordes horizontales */
    for (int i = 1; i < w-1; i++) {
        mvaddch(y,     x+i, ACS_HLINE);
        mvaddch(y+h-1, x+i, ACS_HLINE);
    }
    /* bordes verticales */
    for (int i = 1; i < h-1; i++) {
        mvaddch(y+i, x,     ACS_VLINE);
        mvaddch(y+i, x+w-1, ACS_VLINE);
    }
}

int is_on_platform(int px, int py, int width) {
    for (int i = 0; i < num_platforms; i++) {
        if (py == platforms[i][0] - 1 &&
            px + width - 1 >= platforms[i][1] &&
            px <= platforms[i][2]) {
            return 1;
        }
    }
    return 0;
}

/* ===================== PANTALLA: MENÚ ===================== */
int menu_screen(void) {
    int sel = 0;
    const char *opts[] = {
        "  [1] Iniciar Partida  ",
        "  [2] Instrucciones    ",
        "  [3] Puntajes         ",
        "  [4] Salir            "
    };
    int n = 4;

    while (1) {
        clear();
        attron(COLOR_PAIR(3) | A_BOLD);
        draw_centered(2, "  ____  __  __    _    ____  ___ ___    ____ _____ ");
        draw_centered(3, " / ___||  \\/  |  / \\  |  _ \\|_ _/ _ \\  |  _ \\_   _|");
        draw_centered(4, " \\___ \\| |\\/| | / _ \\ | |_) || | | | | | |_) || |  ");
        draw_centered(5, "  ___) | |  | |/ ___ \\|  _ < | | |_| | |  _ < | |  ");
        draw_centered(6, " |____/|_|  |_/_/   \\_\\_| \\_\\___\\___/  |_| \\_\\|_|  ");
        attroff(COLOR_PAIR(3) | A_BOLD);

        attron(COLOR_PAIR(2));
        draw_centered(7, "  ____  ____   ___  ____    __  __    _    ____  ___ ___   ");
        draw_centered(8, " / ___||  _ \\ / _ \\/ ___|  |  \\/  |  / \\  |  _ \\|_ _/ _ \\ ");
        draw_centered(9, " \\___ \\| |_) | | | \\___ \\  | |\\/| | / _ \\ | |_) || | | | |");
        draw_centered(10,"  ___) |  __/| |_| |___) | | |  | |/ ___ \\|  _ < | | |_| |");
        draw_centered(11," |____/|_|    \\___/|____/  |_|  |_/_/   \\_\\_| \\_\\___\\___/ ");
        attroff(COLOR_PAIR(2));

        /* ASCII Mario decorativo */
        attron(COLOR_PAIR(1));
        draw_centered(13, "  ( ^>)  [?]  (oo)  ||==||  ======  o  [#]  ~o~  |P ");
        attroff(COLOR_PAIR(1));

        /* Opciones */
        for (int i = 0; i < n; i++) {
            if (i == sel) {
                attron(COLOR_PAIR(4) | A_REVERSE | A_BOLD);
            } else {
                attron(COLOR_PAIR(1));
            }
            draw_centered(15 + i, opts[i]);
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

/* ===================== PANTALLA: MODO ===================== */
int mode_select_screen(void) {
    int sel = 0;
    while (1) {
        clear();
        draw_box(3, 15, 18, 50);

        attron(COLOR_PAIR(3) | A_BOLD);
        draw_centered(5, "=== SELECCIONAR MODO DE JUEGO ===");
        attroff(COLOR_PAIR(3) | A_BOLD);

        mvprintw(8, 18, "Selecciona la dificultad:");

        /* Modo 1 */
        if (sel == 0) attron(COLOR_PAIR(4) | A_REVERSE | A_BOLD);
        else          attron(COLOR_PAIR(2));
        mvprintw(11, 20, "  MODO 1 - LENTO  ");
        attroff(COLOR_PAIR(4) | A_REVERSE | A_BOLD);
        attroff(COLOR_PAIR(2));

        attron(COLOR_PAIR(1));
        mvprintw(12, 20, "  Velocidad moderada, mas tiempo");
        mvprintw(13, 20, "  de reaccion. Ideal para aprender.");
        attroff(COLOR_PAIR(1));

        /* Modo 2 */
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

/* ===================== PANTALLA: INSTRUCCIONES ===================== */
void instructions_screen(void) {
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

/* ===================== PANTALLA: PUNTAJES ===================== */
void scores_screen(void) {
    clear();
    draw_box(2, 15, 20, 50);

    attron(COLOR_PAIR(3) | A_BOLD);
    draw_centered(4, "*** MEJORES PUNTAJES ***");
    attroff(COLOR_PAIR(3) | A_BOLD);

    mvprintw(6, 20, "  #    Jugador          Puntaje");
    mvprintw(7, 20, "  --------------------------------");

    if (num_scores == 0) {
        attron(COLOR_PAIR(5));
        draw_centered(12, "  No hay puntajes registrados aun.");
        attroff(COLOR_PAIR(5));
    } else {
        for (int i = 0; i < num_scores; i++) {
            if (i == 0) attron(COLOR_PAIR(3) | A_BOLD);
            else        attron(COLOR_PAIR(1));
            mvprintw(8 + i, 20, "  %d    %-15s  %7d",
                     i+1, high_scores[i].name, high_scores[i].score);
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

/* ===================== INICIALIZAR JUEGO ===================== */
void init_game(void) {
    mario.x         = 5;
    mario.y         = GROUND_ROW - 1;
    mario.vy        = 0;
    mario.on_ground = 1;
    mario.state     = 0;
    mario.invincible= 0;
    mario.lives     = 3;
    mario.score     = 0;
    mario.coins     = 0;
    mario.has_fire  = 0;
    mario.dir       = 1;
    level_time      = 300;

    /* Goombas */
    int ex[] = {20, 35, 50, 65, 28};
    for (int i = 0; i < MAX_ENEMIES; i++) {
        enemies[i].x     = ex[i];
        enemies[i].y     = GROUND_ROW - 1;
        enemies[i].alive = 1;
        enemies[i].dir   = (i % 2 == 0) ? -1 : 1;
        enemies[i].type  = (i < 3) ? 0 : 1; /* 0=goomba, 1=koopa */
    }

    /* Monedas */
    int cx[] = {8,12,18,24,32,40,48,55,63,70};
    for (int i = 0; i < MAX_COINS; i++) {
        coins[i].x = cx[i];
        coins[i].y = GROUND_ROW - 2;
        coins[i].active = 1;
    }

    /* Proyectiles */
    for (int i = 0; i < MAX_PROJECTILES; i++) {
        projectiles[i].active = 0;
    }

    /* Resetear bloques */
    for (int i = 0; i < num_qblocks; i++) qblock_state[i] = 0;
}

/* ===================== HUD ===================== */
void draw_hud(void) {
    attron(COLOR_PAIR(6) | A_BOLD);
    mvprintw(0, 0,
        " MARIO  SCORE:%-7d  LIVES:%d  COINS:%-3d  TIME:%-4d  MODO:%d  HI:%d ",
        mario.score, mario.lives, mario.coins, level_time,
        game_mode, num_scores > 0 ? high_scores[0].score : 0);
    attroff(COLOR_PAIR(6) | A_BOLD);
}

/* ===================== DIBUJO DEL TABLERO ===================== */
void draw_board(void) {
    /* Fondo / cielo */
    attron(COLOR_PAIR(7));
    for (int r = 1; r < BOARD_HEIGHT; r++)
        for (int c = 0; c < BOARD_WIDTH; c++)
            mvaddch(r, c, ' ');
    attroff(COLOR_PAIR(7));

    /* Plataformas */
    attron(COLOR_PAIR(2) | A_BOLD);
    for (int i = 0; i < num_platforms; i++) {
        int row = platforms[i][0];
        int c0  = platforms[i][1];
        int c1  = platforms[i][2];
        for (int c = c0; c <= c1; c++)
            mvprintw(row, c, "=");
    }
    attroff(COLOR_PAIR(2) | A_BOLD);

    /* Bloques [?] */
    for (int i = 0; i < num_qblocks; i++) {
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

    /* Tubos */
    attron(COLOR_PAIR(2));
    for (int i = 0; i < num_pipes; i++) {
        int r = pipes[i][0];
        int c = pipes[i][1];
        mvprintw(r - 1, c, "||==||");
        mvprintw(r,     c, "||  ||");
        mvprintw(r + 1, c, "||  ||");
    }
    attroff(COLOR_PAIR(2));

    /* Monedas */
    attron(COLOR_PAIR(3));
    for (int i = 0; i < MAX_COINS; i++) {
        if (coins[i].active)
            mvprintw(coins[i].y, coins[i].x, "o");
    }
    attroff(COLOR_PAIR(3));

    /* Bandera meta */
    attron(COLOR_PAIR(3) | A_BOLD);
    mvprintw(GROUND_ROW - 5, 76, "|P");
    mvprintw(GROUND_ROW - 4, 76, "| ");
    mvprintw(GROUND_ROW - 3, 76, "| ");
    mvprintw(GROUND_ROW - 2, 76, "| ");
    mvprintw(GROUND_ROW - 1, 76, "| ");
    attroff(COLOR_PAIR(3) | A_BOLD);

    /* Enemigos */
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!enemies[i].alive) continue;
        attron(COLOR_PAIR(1) | A_BOLD);
        if (enemies[i].type == 0) { /* Goomba */
            mvprintw(enemies[i].y - 1, enemies[i].x, "(oo)");
            mvprintw(enemies[i].y,     enemies[i].x, "/mm\\");
        } else if (enemies[i].type == 1) { /* Koopa */
            mvprintw(enemies[i].y - 1, enemies[i].x, "(^^)");
            mvprintw(enemies[i].y,     enemies[i].x, "[==]");
        } else { /* Caparazón */
            mvprintw(enemies[i].y, enemies[i].x, "[==]");
        }
        attroff(COLOR_PAIR(1) | A_BOLD);
    }

    /* Proyectiles */
    attron(COLOR_PAIR(4) | A_BOLD);
    for (int i = 0; i < MAX_PROJECTILES; i++) {
        if (projectiles[i].active)
            mvprintw(projectiles[i].y, projectiles[i].x, "~o~");
    }
    attroff(COLOR_PAIR(4) | A_BOLD);

    /* Mario */
    attron(COLOR_PAIR(4) | A_BOLD);
    if (mario.state == 0) {
        if (mario.dir == 1)
            mvprintw(mario.y, mario.x, "( ^>)");
        else
            mvprintw(mario.y, mario.x, "(<^ )");
    } else { /* Super Mario */
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

/* ===================== HILO: TEMPORIZADOR ===================== */
void *timer_thread(void *arg) {
    (void)arg;
    while (game_running) {
        usleep(1000000);
        pthread_mutex_lock(&board_mutex);
        if (!game_paused && level_time > 0)
            level_time--;
        pthread_mutex_unlock(&board_mutex);
    }
    return NULL;
}

/* ===================== HILO: ENEMIGOS ===================== */
void *enemies_thread(void *arg) {
    (void)arg;
    int speed_us = (game_mode == 1) ? 300000 : 150000;
    while (game_running) {
        usleep(speed_us);
        pthread_mutex_lock(&board_mutex);
        if (!game_paused) {
            for (int i = 0; i < MAX_ENEMIES; i++) {
                if (!enemies[i].alive) continue;
                enemies[i].x += enemies[i].dir;
                /* Rebotar en bordes */
                if (enemies[i].x <= 1 || enemies[i].x >= BOARD_WIDTH - 5)
                    enemies[i].dir *= -1;
                /* Gravedad simple */
                int on_p = 0;
                for (int p = 0; p < num_platforms; p++) {
                    if (enemies[i].y == platforms[p][0] - 1 &&
                        enemies[i].x >= platforms[p][1] &&
                        enemies[i].x <= platforms[p][2]) {
                        on_p = 1; break;
                    }
                }
                if (!on_p && enemies[i].y < GROUND_ROW - 1)
                    enemies[i].y++;
            }
        }
        pthread_mutex_unlock(&board_mutex);
    }
    return NULL;
}

/* ===================== HILO: PROYECTILES ===================== */
void *projectiles_thread(void *arg) {
    (void)arg;
    while (game_running) {
        usleep(80000);
        pthread_mutex_lock(&board_mutex);
        if (!game_paused) {
            for (int i = 0; i < MAX_PROJECTILES; i++) {
                if (!projectiles[i].active) continue;
                projectiles[i].x += 2 * projectiles[i].dir;
                if (projectiles[i].x < 1 || projectiles[i].x > BOARD_WIDTH - 4)
                    projectiles[i].active = 0;
                /* Colisión con enemigos */
                for (int e = 0; e < MAX_ENEMIES; e++) {
                    if (!enemies[e].alive) continue;
                    if (abs(projectiles[i].x - enemies[e].x) < 4 &&
                        abs(projectiles[i].y - enemies[e].y) < 2) {
                        enemies[e].alive = 0;
                        projectiles[i].active = 0;
                        mario.score += 150;
                        break;
                    }
                }
            }
        }
        pthread_mutex_unlock(&board_mutex);
    }
    return NULL;
}

/* ===================== LÓGICA MARIO ===================== */
void update_mario(int key) {
    /* Movimiento horizontal */
    if (key == KEY_LEFT && mario.x > 1) {
        mario.x--;
        mario.dir = -1;
    }
    if (key == KEY_RIGHT && mario.x < BOARD_WIDTH - 6) {
        mario.x++;
        mario.dir = 1;
    }
    /* Salto */
    if ((key == KEY_UP || key == ' ') && mario.on_ground) {
        mario.vy = -4;
        mario.on_ground = 0;
    }
    /* Lanzar proyectil */
    if ((key == 'z' || key == 'Z' || key == 'a' || key == 'A') && mario.has_fire) {
        for (int i = 0; i < MAX_PROJECTILES; i++) {
            if (!projectiles[i].active) {
                projectiles[i].x = mario.x + (mario.dir == 1 ? 5 : -3);
                projectiles[i].y = mario.y;
                projectiles[i].dir = mario.dir;
                projectiles[i].active = 1;
                break;
            }
        }
    }

    /* Física vertical */
    mario.y += mario.vy;
    if (mario.vy < 3) mario.vy++;

    /* Colisión con suelo y plataformas */
    mario.on_ground = 0;
    if (mario.y >= GROUND_ROW - 1) {
        mario.y = GROUND_ROW - 1;
        mario.vy = 0;
        mario.on_ground = 1;
    }
    for (int p = 0; p < num_platforms; p++) {
        if (mario.y == platforms[p][0] - 1 &&
            mario.x + 4 >= platforms[p][1] &&
            mario.x <= platforms[p][2] && mario.vy >= 0) {
            mario.y = platforms[p][0] - 1;
            mario.vy = 0;
            mario.on_ground = 1;
        }
    }
    if (mario.y < 1) mario.y = 1;

    /* Recolectar monedas */
    for (int i = 0; i < MAX_COINS; i++) {
        if (!coins[i].active) continue;
        if (abs(mario.x - coins[i].x) < 5 && abs(mario.y - coins[i].y) < 2) {
            coins[i].active = 0;
            mario.score += 100;
            mario.coins++;
            if (mario.coins >= 100) { mario.lives++; mario.coins = 0; }
        }
    }

    /* Colisión con enemigos */
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!enemies[i].alive) continue;
        int dx = abs(mario.x - enemies[i].x);
        int dy = mario.y - enemies[i].y;
        if (dx < 4) {
            if (dy == -1 && mario.vy > 0) { /* Pisó enemigo */
                enemies[i].alive = 0;
                mario.score += 100;
                mario.vy = -2;
            } else if (abs(dy) < 2) { /* Golpe lateral */
                if (!mario.invincible) {
                    if (mario.state > 0) {
                        mario.state = 0;
                        mario.has_fire = 0;
                    } else {
                        mario.lives--;
                        mario.x = 5; mario.y = GROUND_ROW - 1;
                        mario.vy = 0; mario.on_ground = 1;
                    }
                }
            }
        }
    }

    /* Golpear bloque [?] desde abajo */
    for (int i = 0; i < num_qblocks; i++) {
        if (qblock_state[i]) continue;
        if (abs(mario.x - qblocks[i][1]) < 4 &&
            mario.y - 1 == qblocks[i][0] && mario.vy < 0) {
            qblock_state[i] = 1;
            mario.score += 50;
            /* Soltar hongo (efecto visual simple) */
            mario.state = 1;
        }
    }
}

/* ===================== PANTALLA DE PAUSA ===================== */
void draw_pause(void) {
    attron(COLOR_PAIR(6) | A_BOLD | A_REVERSE);
    draw_centered(10, "                                    ");
    draw_centered(11, "          === JUEGO PAUSADO ===     ");
    draw_centered(12, "      Presiona P para continuar     ");
    draw_centered(13, "                                    ");
    attroff(COLOR_PAIR(6) | A_BOLD | A_REVERSE);
}

/* ===================== PANTALLA: RESULTADO ===================== */
void result_screen(int won) {
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

    /* Registrar puntaje */
    if (num_scores < MAX_SCORES || mario.score > high_scores[num_scores-1].score) {
        /* Insertar puntaje */
        int idx = num_scores < MAX_SCORES ? num_scores : MAX_SCORES - 1;
        if (num_scores < MAX_SCORES) num_scores++;
        high_scores[idx].score = mario.score + (won ? level_time * 50 : 0);
        snprintf(high_scores[idx].name, 16, "MARIO-%d", idx+1);
        /* Ordenar (bubble) */
        for (int i = 0; i < num_scores - 1; i++)
            for (int j = i+1; j < num_scores; j++)
                if (high_scores[j].score > high_scores[i].score) {
                    HighScore tmp = high_scores[i];
                    high_scores[i] = high_scores[j];
                    high_scores[j] = tmp;
                }
    }

    attron(COLOR_PAIR(5));
    mvprintw(16, 20, "  [R] Reiniciar   [Q/ESC] Salir");
    attroff(COLOR_PAIR(5));

    refresh();
    while (1) {
        int ch = getch();
        if (ch == 'r' || ch == 'R') break;
        if (ch == 'q' || ch == 'Q' || ch == 27) { game_running = 0; break; }
    }
    nodelay(stdscr, TRUE);
}

/* ===================== LOOP PRINCIPAL ===================== */
void game_loop(void) {
    init_game();
    game_running = 1;
    game_paused  = 0;

    pthread_t t_timer, t_enemies, t_proj;
    pthread_create(&t_timer,   NULL, timer_thread,       NULL);
    pthread_create(&t_enemies, NULL, enemies_thread,     NULL);
    pthread_create(&t_proj,    NULL, projectiles_thread, NULL);

    int won = 0;
    while (game_running) {
        /* Entrada no bloqueante */
        int ch = getch();
        if (ch == 'q' || ch == 'Q' || ch == 27) { game_running = 0; break; }
        if (ch == 'p' || ch == 'P') {
            game_paused ^= 1;
        }

        if (!game_paused) {
            pthread_mutex_lock(&board_mutex);
            update_mario(ch);
            pthread_mutex_unlock(&board_mutex);
        }

        /* Detectar Game Over */
        if (mario.lives <= 0 || level_time <= 0) {
            game_running = 0; break;
        }
        /* Detectar victoria (llegar a bandera) */
        if (mario.x >= 75) {
            won = 1; game_running = 0; break;
        }

        /* Dibujar */
        erase();
        draw_hud();
        pthread_mutex_lock(&board_mutex);
        draw_board();
        pthread_mutex_unlock(&board_mutex);
        if (game_paused) draw_pause();
        refresh();

        usleep(40000); /* ~25 FPS */
    }

    pthread_join(t_timer,   NULL);
    pthread_join(t_enemies, NULL);
    pthread_join(t_proj,    NULL);

    if (mario.lives > 0 || won)
        result_screen(won);
    else
        result_screen(0);
}

/* ===================== MAIN ===================== */
int main(void) {
    /* Iniciar ncurses */
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
    nodelay(stdscr, TRUE);

    /* Colores */
    if (has_colors()) {
        start_color();
        use_default_colors();
        init_pair(1, COLOR_WHITE,   -1); /* elementos normales */
        init_pair(2, COLOR_GREEN,   -1); /* plataformas */
        init_pair(3, COLOR_YELLOW,  -1); /* bloques / vittoria */
        init_pair(4, COLOR_RED,     -1); /* mario / proyectiles */
        init_pair(5, COLOR_CYAN,    -1); /* info / pie de pagina */
        init_pair(6, COLOR_BLACK,   COLOR_WHITE); /* HUD */
        init_pair(7, COLOR_BLUE,    -1); /* cielo */
    }

    srand((unsigned)time(NULL));

    /* Puntajes de ejemplo */
    num_scores = 3;
    strcpy(high_scores[0].name, "LUIGI");   high_scores[0].score = 12500;
    strcpy(high_scores[1].name, "PEACH");   high_scores[1].score =  8200;
    strcpy(high_scores[2].name, "TOAD");    high_scores[2].score =  5500;

    /* Bucle menú */
    while (1) {
        int opt = menu_screen();
        if (opt == 0) {
            int mode = mode_select_screen();
            if (mode > 0) {
                game_mode = mode;
                game_loop();
            }
        } else if (opt == 1) {
            instructions_screen();
        } else if (opt == 2) {
            scores_screen();
        } else {
            break;
        }
    }

    endwin();
    return 0;
}
