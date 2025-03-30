#pragma once
#include "constants.h"
#include <SDL.h>
#include <SDL_image.h>
#include <iostream>


class Bullet {
public:
  float x, y, dx, dy;
  SDL_Rect rect;
  bool active;
  SDL_Renderer *renderer;
  SDL_Texture *textureUp, *textureDown, *textureLeft, *textureRight,
      *currentTexture;

  Bullet(int startX, int startY, int dirX, int dirY, SDL_Renderer *ren);
  ~Bullet();
  void update();
  void render();
};
