
#pragma once
#include "Bullet.h"
#include "EnemyTank.h"
#include "PlayTank.h"
#include "Wall.h"
#include "constants.h"
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <vector>


class Game {
public:
  SDL_Window *window;
  SDL_Renderer *renderer;
  SDL_Texture *wallTexture;
  SDL_Texture *whiteBrickTexture;
  SDL_Texture *waterTexture;
  SDL_Texture *gameOverTexture;
  SDL_Texture *youWinTexture;
  SDL_Texture *menuTexture;
  bool running;
  std::vector<Wall> walls;
  std::vector<Bullet> bullets;
  PlayTank *player;
  std::vector<EnemyTank> enemies;
  enum GameState { PLAYING, WIN, LOSE, MENU };
  GameState gameState;
  Uint32 gameOverStartTime;
  Uint32 winStartTime;
  Mix_Chunk *boomSound;

  Game();
  ~Game();
  void generateWalls();
  void generateRandomEnemies();
  void updateBullets();
  void updateEnemyBullets();
  void updateEnemies();
  void renderGameOver();
  void renderYouWin();
  void renderMenu();
  void handleMenuEvents(SDL_Event &e);
  void run();
};
