
#pragma once
#include <SDL.h>

class Wall {
public:
  int x, y;
  SDL_Rect rect;
  SDL_Texture *texture;
  int type; // 0: red_brick, 1: white_brick, 2: water

  Wall(int startX, int startY, SDL_Texture *tex, int wallType = 0);
  void render(SDL_Renderer *renderer);
};
