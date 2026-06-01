#include "Renderer.h"
#include "Constantes.h"
#include "Level.h"
#include "Entidades/MarioPlayer.h"
#include "Entidades/Enemy.h"
#include "Entidades/Coin.h"
#include "Entidades/Projectile.h"

void Renderer::draw_board(const MarioPlayer& mario, const Level& level, const std::vector<Enemy>& enemies, const std::vector<Coin>& coins, const std::vector<Projectile>& projectiles) 
{
        // Fondo / cielo
        attron(COLOR_PAIR(7));
        for (int r = 1; r < BOARD_HEIGHT; r++)
            for (int c = 0; c < BOARD_WIDTH; c++)
                mvaddch(r, c, ' ');
        attroff(COLOR_PAIR(7));

        // Plataformas
        attron(COLOR_PAIR(2) | A_BOLD);
        for (const auto &plat : level.platforms) {
            int row = plat[0];
            int c0  = plat[1];
            int c1  = plat[2];
            for (int c = c0; c <= c1; c++)
                mvprintw(row, c, "=");
        }
        attroff(COLOR_PAIR(2) | A_BOLD);

        // Bloques [?]
        for (size_t i = 0; i < level.qblocks.size(); i++) {
            if (level.qblock_state[i] == 0) {
                attron(COLOR_PAIR(3) | A_BOLD);
                mvprintw(level.qblocks[i][0], level.qblocks[i][1], "[?]");
                attroff(COLOR_PAIR(3) | A_BOLD);
            } else {
                attron(COLOR_PAIR(5));
                mvprintw(level.qblocks[i][0], level.qblocks[i][1], "[*]");
                attroff(COLOR_PAIR(5));
            }
        }

        // Tubos
        attron(COLOR_PAIR(2) | A_BOLD);
        for (const auto &pipe : level.pipes) {
            int r = pipe[0];
            int c = pipe[1];
            mvprintw(r - 1, c, "||==||");
            mvprintw(r,     c, "||  ||");
            mvprintw(r + 1, c, "||  ||");
        }
        attroff(COLOR_PAIR(2) | A_BOLD);

        // Monedas
        attron(COLOR_PAIR(3) | A_BOLD);
        for (const auto &coin : coins) {
            if (coin.active)
                mvprintw(coin.y, coin.x, "o");
        }
        attroff(COLOR_PAIR(3) | A_BOLD);

        // Bandera meta
        attron(COLOR_PAIR(3) | A_BOLD);
        for(int i = 5; i >= 1; i--) {
            mvprintw(GROUND_ROW - i, 76, (i == 5) ? "|P" : "| ");
        }
        attroff(COLOR_PAIR(3) | A_BOLD);

        // Enemigos
        for (const auto &enemy : enemies) {
            if (!enemy.alive) continue;
            if (enemy.type == 0) {
                attron(COLOR_PAIR(8) | A_BOLD);
                mvprintw(enemy.y - 1, enemy.x, "(oo)");
                mvprintw(enemy.y,     enemy.x, "/mm\\");
                attroff(COLOR_PAIR(8) | A_BOLD);
            } else if (enemy.type == 1) {
                attron(COLOR_PAIR(2) | A_BOLD);
                mvprintw(enemy.y - 1, enemy.x, "(^^)");
                mvprintw(enemy.y,     enemy.x, "[==]");
                attroff(COLOR_PAIR(2) | A_BOLD);
            } else {
                attron(COLOR_PAIR(1) | A_BOLD);
                mvprintw(enemy.y, enemy.x, "[==]");
                attroff(COLOR_PAIR(1) | A_BOLD);
            }
        }

        // Proyectiles
        attron(COLOR_PAIR(3) | A_BOLD);
        for (const auto &proj : projectiles) {
            if (proj.active)
                mvprintw(proj.y, proj.x, "~o~");
        }
        attroff(COLOR_PAIR(3) | A_BOLD);

        // Mario
        if (mario.state == 0) {
            attron(COLOR_PAIR(4) | A_BOLD);
            if (mario.dir == 1)  mvprintw(mario.y, mario.x, "( ^>)");
            else                 mvprintw(mario.y, mario.x, "(<^ )");
            attroff(COLOR_PAIR(4) | A_BOLD);
        } else {
            attron(COLOR_PAIR(4) | A_BOLD);
            if (mario.dir == 1)  mvprintw(mario.y - 1, mario.x, "( ^>)");
            else                 mvprintw(mario.y - 1, mario.x, "(<^ )");
            attroff(COLOR_PAIR(4) | A_BOLD);
            attron(COLOR_PAIR(7) | A_BOLD);
            mvprintw(mario.y, mario.x, "( || )");
            attroff(COLOR_PAIR(7) | A_BOLD);
        }
}
        

void Renderer::draw_hud(const MarioPlayer& mario, int level_time, int game_mode,int hi_score) {
        attron(COLOR_PAIR(6) | A_BOLD);
        mvprintw(0, 0,
            " MARIO  SCORE:%-7d  LIVES:%d  COINS:%-3d  TIME:%-4d  MODO:%d  HI:%d ",
            mario.score, mario.lives, mario.coins, level_time,
            game_mode, hi_score);
        attroff(COLOR_PAIR(6) | A_BOLD);
    }

void Renderer::draw_pause() {
        attron(COLOR_PAIR(6) | A_BOLD | A_REVERSE);
        draw_centered(10, "                                    ");
        draw_centered(11, "          === JUEGO PAUSADO ===     ");
        draw_centered(12, "      Presiona P para continuar     ");
        draw_centered(13, "                                    ");
        attroff(COLOR_PAIR(6) | A_BOLD | A_REVERSE);
    }

void Renderer::draw_box(int y, int x, int h, int w) {
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

void Renderer::draw_centered(int row, const std::string &str) {
        int col = (BOARD_WIDTH - str.length()) / 2;
        if (col < 0) col = 0;
        mvprintw(row, col, "%s", str.c_str());
    }