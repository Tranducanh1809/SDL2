
#include "PlayTank.h"

PlayTank::PlayTank(int startX, int startY, SDL_Renderer *ren,
                   std::vector<Wall> *wallList, std::vector<Bullet> *bulletList)
    : x(startX), y(startY), renderer(ren), walls(wallList), bullets(bulletList),
      dirX(0), dirY(-1) {
  rect = {x, y, TILE_SIZE, TILE_SIZE};
  textureUp = IMG_LoadTexture(renderer, "yellow_up.png");
  textureDown = IMG_LoadTexture(renderer, "yellow_down.png");
  textureLeft = IMG_LoadTexture(renderer, "yellow_left.png");
  textureRight = IMG_LoadTexture(renderer, "yellow_right.png");
  currentTexture = textureUp;

  // Load shooting sound
  shootSound = Mix_LoadWAV("bullet_sound.wav");
  if (!shootSound) {
    std::cout << "Failed to load shoot sound: " << Mix_GetError() << std::endl;
  }
}

PlayTank::~PlayTank() {
  if (shootSound) {
    Mix_FreeChunk(shootSound);
  }
}

bool PlayTank::checkCollision(int newX, int newY,
                              const std::vector<Wall> &walls) {
  SDL_Rect newRect = {newX, newY, TILE_SIZE, TILE_SIZE};
  for (const auto &wall : walls) {
    if (SDL_HasIntersection(&newRect, &wall.rect))
      return true; // Kiểm tra va chạm với tất cả các loại tường
  }
  return false;
}

void PlayTank::handleEvent(SDL_Event &e) {
  if (e.type == SDL_KEYDOWN) {
    int newX = x, newY = y;
    switch (e.key.keysym.sym) {
    case SDLK_UP:
      newY -= TILE_SIZE;
      currentTexture = textureUp;
      dirX = 0;
      dirY = -1;
      break;
    case SDLK_DOWN:
      newY += TILE_SIZE;
      currentTexture = textureDown;
      dirX = 0;
      dirY = 1;
      break;
    case SDLK_LEFT:
      newX -= TILE_SIZE;
      currentTexture = textureLeft;
      dirX = -1;
      dirY = 0;
      break;
    case SDLK_RIGHT:
      newX += TILE_SIZE;
      currentTexture = textureRight;
      dirX = 1;
      dirY = 0;
      break;
    case SDLK_SPACE:
      bullets->emplace_back(x + TILE_SIZE / 2 - 8, y + TILE_SIZE / 2 - 8, dirX,
                            dirY, renderer);
      if (shootSound) {
        Mix_PlayChannel(-1, shootSound, 0); // Play shooting sound
      }
      break; // Sửa vị trí bắn
    }
    if (!checkCollision(newX, newY, *walls) && newX >= 0 && newY >= 0 &&
        newX + TILE_SIZE <= SCREEN_WIDTH && newY + TILE_SIZE <= SCREEN_HEIGHT) {
      x = newX;
      y = newY;
      rect.x = x;
      rect.y = y;
    }
  }
}

void PlayTank::render() {
  if (currentTexture) {
    SDL_RenderCopy(renderer, currentTexture, nullptr, &rect);
  }
}
