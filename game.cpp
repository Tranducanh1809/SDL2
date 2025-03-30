#include "Game.h"
#include <algorithm>
#include <ctime>


Game::Game() {
  running = true;
  SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO); // Initialize SDL audio
  window = SDL_CreateWindow("Battle City", SDL_WINDOWPOS_CENTERED,
                            SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT,
                            SDL_WINDOW_SHOWN);
  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

  // Initialize SDL_mixer
  if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
    std::cout << "SDL_mixer could not initialize! SDL_mixer Error: "
              << Mix_GetError() << std::endl;
  }
  boomSound = Mix_LoadWAV("boom_sound.wav");
  if (!boomSound) {
    std::cout << "Failed to load boom sound: " << Mix_GetError() << std::endl;
  }

  wallTexture = IMG_LoadTexture(renderer, "red_brick.png");
  whiteBrickTexture = IMG_LoadTexture(renderer, "white_brick.png");
  waterTexture = IMG_LoadTexture(renderer, "water.png");
  gameOverTexture = IMG_LoadTexture(renderer, "game_over.png");
  youWinTexture = IMG_LoadTexture(renderer, "you_win.png");
  menuTexture = IMG_LoadTexture(renderer, "menu.png");
  gameState = MENU;
  srand(std::time(0));
}

Game::~Game() {
  Mix_FreeChunk(boomSound);
  Mix_CloseAudio(); // Close SDL_mixer
}

void Game::generateWalls() {
  for (int i = 0; i < MAP_HEIGHT; i++) {
    for (int j = 0; j < MAP_WIDTH; j++) {
      if (i == 0 || i == MAP_HEIGHT - 1 || j == 0 || j == MAP_WIDTH - 1) {
        walls.emplace_back(j * TILE_SIZE, i * TILE_SIZE, wallTexture);
      }
    }
  }

  std::string mapLayout[7] = {"CCBBBW   WCCBBC ", "B    B   BB    ",
                              "B    B   C    ",   "WBCCB    W    ",
                              "B    B   C    ",   "B   WB   CBW   ",
                              "CCBBBW   BBCCBB "};

  int startX = MAP_WIDTH / 2 - 7;
  int startY = MAP_HEIGHT / 2 - 3;

  for (int i = 0; i < 7; i++) {
    for (int j = 0; j < 14; j++) {
      if (mapLayout[i][j] == 'B') {
        walls.emplace_back((startX + j) * TILE_SIZE, (startY + i) * TILE_SIZE,
                           wallTexture);
      } else if (mapLayout[i][j] == 'C') {
        walls.emplace_back((startX + j) * TILE_SIZE, (startY + i) * TILE_SIZE,
                           whiteBrickTexture, 1);
      } else if (mapLayout[i][j] == 'W') {
        walls.emplace_back((startX + j) * TILE_SIZE, (startY + i) * TILE_SIZE,
                           waterTexture, 2);
      }
    }
  }
}

void Game::generateRandomEnemies() {
  for (int i = 0; i < 4; ++i) {
    bool validPosition = false;
    int randomX, randomY;

    while (!validPosition) {
      randomX = (rand() % (MAP_WIDTH - 2)) * TILE_SIZE + TILE_SIZE;
      randomY = (rand() % (MAP_HEIGHT - 2)) * TILE_SIZE + TILE_SIZE;

      SDL_Rect enemyRect = {randomX, randomY, TILE_SIZE, TILE_SIZE};
      validPosition = true;

      for (const auto &wall : walls) {
        if (SDL_HasIntersection(&enemyRect, &wall.rect)) {
          validPosition = false;
          break;
        }
      }

      for (const auto &enemy : enemies) {
        if (SDL_HasIntersection(&enemyRect, &enemy.rect)) {
          validPosition = false;
          break;
        }
      }

      if (player && SDL_HasIntersection(&enemyRect, &player->rect)) {
        validPosition = false;
        break;
      }
    }
    enemies.emplace_back(randomX, randomY, renderer);
  }
}

void Game::updateBullets() {
  for (auto &bullet : bullets) {
    bullet.update();
  }

  bullets.erase(
      std::remove_if(bullets.begin(), bullets.end(),
                     [&](Bullet &b) {
                       for (auto it = walls.begin(); it != walls.end(); ++it) {
                         if (it->type == 0 &&
                             SDL_HasIntersection(&b.rect, &it->rect)) {
                           walls.erase(it);
                           if (boomSound) {
                             Mix_PlayChannel(-1, boomSound, 0);
                           }
                           return true;
                         } else if (it->type == 1 &&
                                    SDL_HasIntersection(&b.rect, &it->rect)) {
                           if (boomSound) {
                             Mix_PlayChannel(-1, boomSound, 0);
                           }
                           return true;
                         }
                       }
                       for (size_t j = 0; j < enemies.size(); ++j) {
                         EnemyTank &enemy = enemies[j];
                         if (SDL_HasIntersection(&b.rect, &enemy.rect)) {
                           enemies.erase(enemies.begin() + j);
                           if (boomSound) {
                             Mix_PlayChannel(-1, boomSound, 0);
                           }
                           return true;
                         }
                       }
                       return !b.active;
                     }),
      bullets.end());

  if (enemies.empty() && gameState == PLAYING) {
    gameState = WIN;
    running = false;
  }
}

void Game::updateEnemyBullets() {
  for (auto &enemy : enemies) {
    enemy.updateBullets(walls, enemies);
    for (auto &bullet : enemy.bullets) {
      if (SDL_HasIntersection(&bullet.rect, &player->rect)) {
        gameState = LOSE;
        running = false;
        return;
      }
    }
  }
}

void Game::updateEnemies() {
  for (auto &enemy : enemies) {
    enemy.move(walls, enemies);
    enemy.shoot();

    if (SDL_HasIntersection(&enemy.rect, &player->rect)) {
      gameState = LOSE;
      running = false;
      return;
    }
  }
}

void Game::renderGameOver() {
  if (gameOverTexture) {
    SDL_Rect gameOverRect = {SCREEN_WIDTH / 2 - 200, SCREEN_HEIGHT / 2 - 100,
                             400, 200};
    SDL_RenderCopy(renderer, gameOverTexture, nullptr, &gameOverRect);
  }
}

void Game::renderYouWin() {
  if (youWinTexture) {
    SDL_Rect youWinRect = {SCREEN_WIDTH / 2 - 200, SCREEN_HEIGHT / 2 - 100, 400,
                           200};
    SDL_RenderCopy(renderer, youWinTexture, nullptr, &youWinRect);
  }
}

void Game::renderMenu() {
  if (menuTexture) {
    SDL_RenderCopy(renderer, menuTexture, nullptr, nullptr);
  }
}

void Game::handleMenuEvents(SDL_Event &e) {
  if (e.type == SDL_KEYDOWN) {
    switch (e.key.keysym.sym) {
    case SDLK_RETURN:
      gameState = PLAYING;
      running = true;
      generateWalls();
      player = new PlayTank(SCREEN_WIDTH / 2, SCREEN_HEIGHT - TILE_SIZE * 2,
                            renderer, &walls, &bullets);
      srand(std::time(0));
      generateRandomEnemies();
      break;
    case SDLK_ESCAPE:
      running = false;
      break;
    default:
      break;
    }
  }
}

void Game::run() {
  SDL_Event e;
  while (running) {
    while (SDL_PollEvent(&e)) {
      if (e.type == SDL_QUIT) {
        running = false;
        return;
      }

      if (e.type == SDL_KEYDOWN) {
        if (e.key.keysym.sym == SDLK_ESCAPE) {
          running = false;
          return;
        }
      }

      if (gameState == MENU) {
        handleMenuEvents(e);
      } else {
        player->handleEvent(e);
      }
    }

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    if (gameState == MENU) {
      renderMenu();
    } else if (gameState == PLAYING) {
      updateBullets();
      updateEnemyBullets();
      updateEnemies();

      for (auto &wall : walls)
        wall.render(renderer);

      for (auto &bullet : bullets)
        bullet.render();
      for (auto &enemy : enemies) {
        for (auto &bullet : enemy.bullets) {
          bullet.render();
        }
      }

      player->render();
      for (auto &enemy : enemies)
        enemy.render();
    } else if (gameState == WIN) {
      renderYouWin();
      if (winStartTime == 0) {
        winStartTime = SDL_GetTicks();
      }
      if (SDL_GetTicks() - winStartTime > 1000) {
        gameState = MENU;
        running = false;
      }
    } else if (gameState == LOSE) {
      renderGameOver();
      if (gameOverStartTime == 0) {
        gameOverStartTime = SDL_GetTicks();
      }
      if (SDL_GetTicks() - gameOverStartTime > 1000) {
        gameState = MENU;
        running = false;
      }
    }

    SDL_RenderPresent(renderer);
  }

  if (gameState != MENU) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    if (gameState == WIN) {
      renderYouWin();
    } else if (gameState == LOSE) {
      renderGameOver();
    }

    SDL_RenderPresent(renderer);
    SDL_Delay(1000);
    gameState = MENU;
    running = true;
    walls.clear();
    bullets.clear();
    enemies.clear();
    delete player;
    player = nullptr;
    winStartTime = 0;
    gameOverStartTime = 0;
    run();
  }
}
