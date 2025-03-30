#include "Wall.h"
#include "constants.h"

Wall::Wall(int startX, int startY, SDL_Texture *tex, int wallType)
    : x(startX), y(startY), texture(tex), type(wallType) {
  rect = {x, y, TILE_SIZE, TILE_SIZE};
}

void Wall::render(SDL_Renderer *renderer) {
  if (texture) {
    SDL_RenderCopy(renderer, texture, nullptr, &rect);
  }
}
