#include <thread>
#include <mutex>
#include <cmath>
#include <unistd.h>
#include <vector>
#include <algorithm>
#include "Game.h"
#include "Renderer.h"
#include "Level.h"
#include "Entidades/MarioPlayer.h"


Game::Game() : game_mode(1), game_running(false), game_paused(false), level_time(300) {
        // Inicializar puntajes por defecto
        scores.add_score("LUIGI", 12500);
        scores.add_score("PEACH", 8200);
        scores.add_score("TOAD", 5500);
        projectiles.resize(MAX_PROJECTILES);
        level.qblock_state.resize(level.qblocks.size(), 0);
    }
    
void Game::game_loop() {
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
            renderer.draw_hud(mario, level_time, game_mode, scores.get_scores().empty() ? 0 : scores.get_scores()[0].score);
            {
                std::lock_guard<std::mutex> lock(board_mutex);
                renderer.draw_board(mario, level, enemies, coins, projectiles);
            }
            if (game_paused) renderer.draw_pause();
            refresh();

            usleep(40000); // ~25 FPS
        }

        // Sincronización y cierre correcto de hilos
        if (t_timer.joinable()) t_timer.join();
        if (t_enemies.joinable()) t_enemies.join();
        if (t_proj.joinable()) t_proj.join();

        result_screen(won);
    }

void Game::init_game() {
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
}

void Game::timer_loop() {
    while (game_running) {
        usleep(1000000);
        std::lock_guard<std::mutex> lock(board_mutex);
        if (!game_paused && level_time > 0)
            level_time--;
    }
}

void Game::enemies_loop() {
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
                    for (const auto &plat : level.platforms) {
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

void Game::projectiles_loop() {
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

void Game::update_mario(int key) {
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
        for (const auto &plat : level.platforms) {
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
        for (size_t i = 0; i < level.qblocks.size(); i++) {
            if (level.qblock_state[i]) continue;
            if (std::abs(mario.x - level.qblocks[i][1]) < 4 &&
                mario.y - 1 == level.qblocks[i][0] && mario.vy < 0) {
                level.qblock_state[i] = 1;
                mario.score += 50;
                mario.state = 1; // Crecer a Super Mario
            }
        }
    }

void Game::result_screen(bool won) {
        nodelay(stdscr, FALSE);
        clear();
        renderer.draw_box(4, 15, 16, 50);

        attron(COLOR_PAIR(won ? 3 : 1) | A_BOLD);
        if (won) renderer.draw_centered(6, "*** NIVEL COMPLETADO! ***");
        else     renderer.draw_centered(6, "*** GAME OVER ***");
        attroff(COLOR_PAIR(won ? 3 : 1) | A_BOLD);

        attron(COLOR_PAIR(2));
        renderer.draw_centered(
    9,
    "Puntuacion final : " + std::to_string(mario.score));
        renderer.draw_centered(10, "  Monedas recogidas: " + std::to_string(mario.coins));
        renderer.draw_centered(11, "  Vidas restantes  : " + std::to_string(mario.lives));
        renderer.draw_centered(12, "  Tiempo restante  : " + std::to_string(level_time) + " seg");
        if (won)
            renderer.draw_centered(13, "  Bonus tiempo     : +" + std::to_string(level_time * 50) + " pts");
        attroff(COLOR_PAIR(2));

        // Registrar puntaje al estilo C++ (vectores y algoritmos)
        int final_score = mario.score + (won ? level_time * 50 : 0);
        if (scores.get_scores().size() < MAX_SCORES || final_score > scores.get_scores().back().score) {
            std::string name = "MARIO-" + std::to_string(scores.get_scores().size() + 1);
            scores.add_score(name, final_score);

            // Ordenar de mayor a menor utilizando lambdas de C++
            std::sort(scores.get_scores().begin(), scores.get_scores().end(), [](const HighScore &a, const HighScore &b) {
                return a.score > b.score;
            });

            if (scores.get_scores().size() > MAX_SCORES) {
                scores.get_scores().pop_back();
            }
        }

        attron(COLOR_PAIR(5));
        renderer.draw_centered(16, "  [R] Reiniciar   [Q/ESC] Salir");
        attroff(COLOR_PAIR(5));

        refresh();
        while (true) {
            int ch = getch();
            if (ch == 'r' || ch == 'R') break;
            if (ch == 'q' || ch == 'Q' || ch == 27) { game_running = false; break; }
        }
        nodelay(stdscr, TRUE);
    }

void Game::set_mode(int mode) {
    game_mode = mode;
}
