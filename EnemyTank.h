#pragma once
#include "Bullet.h"
#include "Wall.h"
#include "constants.h"
#include <SDL.h>
#include <SDL_image.h>
#include <vector>


class EnemyTank {
public:
  int x, y;
  int dirX, dirY;
  int moveDelay, shootDelay;
  SDL_Rect rect;
  bool active;
  std::vector<Bullet> bullets;
  SDL_Renderer *renderer;
  SDL_Texture *textureUp, *textureDown, *textureLeft, *textureRight,
      *currentTexture;

  EnemyTank(int startX, int startY, SDL_Renderer *ren);
  bool checkCollision(int newX, int newY, const std::vector<Wall> &walls,
                      const std::vector<EnemyTank> &enemies);
  void move(const std::vector<Wall> &walls,
            const std::vector<EnemyTank> &enemies);
  void shoot();
  void updateBullets(std::vector<Wall> &walls, std::vector<EnemyTank> &enemies);
  void render();
};
