#include "EnemyTank.h"
#include <algorithm>

EnemyTank::EnemyTank(int startX, int startY, SDL_Renderer *ren)
    : renderer(ren) {
  moveDelay = 12;    // Tốc độ di chuyển
  shootDelay = 7000; // Tốc độ bắn

  x = startX;
  y = startY;

  rect = {x, y, TILE_SIZE, TILE_SIZE};
  dirX = 0;
  dirY = 1;
  active = true;

  // Load các texture cho 4 hướng
  textureUp = IMG_LoadTexture(renderer, "green_up.png");
  textureDown = IMG_LoadTexture(renderer, "green_down.png");
  textureLeft = IMG_LoadTexture(renderer, "green_left.png");
  textureRight = IMG_LoadTexture(renderer, "green_right.png");
  currentTexture = textureDown; // Bắt đầu hướng xuống
}

bool EnemyTank::checkCollision(int newX, int newY,
                               const std::vector<Wall> &walls,
                               const std::vector<EnemyTank> &enemies) {
  SDL_Rect newRect = {newX, newY, TILE_SIZE, TILE_SIZE};

  for (const auto &wall : walls) {
    if (SDL_HasIntersection(&newRect, &wall.rect)) {
      return true;
    }
  }

  for (const auto &enemy : enemies) {
    if (this != &enemy && SDL_HasIntersection(&newRect, &enemy.rect)) {
      return true;
    }
  }

  return false;
}

void EnemyTank::move(const std::vector<Wall> &walls,
                     const std::vector<EnemyTank> &enemies) {
  if (--moveDelay > 0)
    return;
  moveDelay = 3000;

  int newX = x;
  int newY = y;

  int randomDirection = rand() % 4;
  switch (randomDirection) {
  case 0:
    newY -= TILE_SIZE;
    dirX = 0;
    dirY = -1;
    currentTexture = textureUp;
    break;
  case 1:
    newY += TILE_SIZE;
    dirX = 0;
    dirY = 1;
    currentTexture = textureDown;
    break;
  case 2:
    newX -= TILE_SIZE;
    dirX = -1;
    dirY = 0;
    currentTexture = textureLeft;
    break;
  case 3:
    newX += TILE_SIZE;
    dirX = 1;
    dirY = 0;
    currentTexture = textureRight;
    break;
  }

  if (!checkCollision(newX, newY, walls, enemies) && newX >= 0 &&
      newX < SCREEN_WIDTH - TILE_SIZE && newY >= 0 &&
      newY < SCREEN_HEIGHT - TILE_SIZE) {
    x = newX;
    y = newY;
    rect.x = x;
    rect.y = y;
  }
}

void EnemyTank::shoot() {
  if (--shootDelay > 0)
    return;
  shootDelay = 5000; // Reset delay
  bullets.emplace_back(x + TILE_SIZE / 2 - 8, y + TILE_SIZE / 2 - 8, dirX, dirY,
                       renderer); // Sửa vị trí bắn
}

void EnemyTank::updateBullets(std::vector<Wall> &walls,
                              std::vector<EnemyTank> &enemies) {
  std::vector<int> bulletsToRemove;

  for (size_t i = 0; i < bullets.size(); ++i) {
    Bullet &b = bullets[i];
    bool removeBullet = false;

    for (auto it = walls.begin(); it != walls.end(); ++it) {
      if (it->type == 0 && SDL_HasIntersection(&b.rect, &it->rect)) {
        walls.erase(it);
        removeBullet = true;
        break;
      } else if (it->type == 1 && SDL_HasIntersection(&b.rect, &it->rect)) {
        removeBullet = true;
        break;
      }
    }

    if (!removeBullet) {
      for (auto &enemy : enemies) {
        if (this != &enemy && SDL_HasIntersection(&b.rect, &enemy.rect)) {
          bulletsToRemove.push_back(i);
          removeBullet = true;
          break;
        }
      }
    }

    if (removeBullet) {
      bulletsToRemove.push_back(i);
    }
  }

  std::sort(bulletsToRemove.rbegin(), bulletsToRemove.rend());
  for (size_t index : bulletsToRemove) {
    if (index < bullets.size()) {
      bullets.erase(bullets.begin() + index);
    }
  }

  for (auto &bullet : bullets) {
    bullet.x += bullet.dx * 0.1;
    bullet.y += bullet.dy * 0.1;
    bullet.rect.x = bullet.x;
    bullet.rect.y = bullet.y;
  }
}

void EnemyTank::render() {
  if (currentTexture) {
    SDL_RenderCopy(renderer, currentTexture, nullptr, &rect);
  }
  for (auto &bullet : bullets) {
    bullet.render();
  }
}
