#pragma once
#include "Bullet.h"
#include "Wall.h"
#include "constants.h"
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <vector>


class PlayTank {
public:
  int x, y;
  SDL_Rect rect;
  SDL_Texture *textureUp, *textureDown, *textureLeft, *textureRight,
      *currentTexture;
  SDL_Renderer *renderer;
  std::vector<Wall> *walls;
  std::vector<Bullet> *bullets;
  int dirX, dirY;
  Mix_Chunk *shootSound;

  PlayTank(int startX, int startY, SDL_Renderer *ren,
           std::vector<Wall> *wallList, std::vector<Bullet> *bulletList);
  ~PlayTank();
  bool checkCollision(int newX, int newY, const std::vector<Wall> &walls);
  void handleEvent(SDL_Event &e);
  void render();
};
