#include "Bullet.h"

Bullet::Bullet(int startX, int startY, int dirX, int dirY, SDL_Renderer *ren)
    : renderer(ren), active(true) {
  x = startX - 5; // Center bullet
  y = startY - 5;
  dx = dirX;
  dy = dirY;
  rect = {x, y, 20, 20};

  // Load the bullet images
  textureUp = IMG_LoadTexture(renderer, "bullet_up.png");
  textureDown = IMG_LoadTexture(renderer, "bullet_down.png");
  textureLeft = IMG_LoadTexture(renderer, "bullet_left.png");
  textureRight = IMG_LoadTexture(renderer, "bullet_right.png");

  if (!textureUp || !textureDown || !textureLeft || !textureRight) {
    std::cout << "Failed to load bullet textures: " << IMG_GetError()
              << std::endl;
  }

  // Set the current texture based on the direction
  if (dirY == -1)
    currentTexture = textureUp;
  else if (dirY == 1)
    currentTexture = textureDown;
  else if (dirX == -1)
    currentTexture = textureLeft;
  else if (dirX == 1)
    currentTexture = textureRight;
}

Bullet::~Bullet() {
  if (textureUp)
    SDL_DestroyTexture(textureUp);
  if (textureDown)
    SDL_DestroyTexture(textureDown);
  if (textureLeft)
    SDL_DestroyTexture(textureLeft);
  if (textureRight)
    SDL_DestroyTexture(textureRight);
}

void Bullet::update() {
  if (active) {
    x += dx * BULLET_SPEED;
    y += dy * BULLET_SPEED;
    rect.x = x;
    rect.y = y;

    if (x < 0 || x > SCREEN_WIDTH || y < 0 || y > SCREEN_HEIGHT) {
      active = false;
    }
  }
}

void Bullet::render() {
  if (active && currentTexture) {
    SDL_RenderCopy(renderer, currentTexture, nullptr, &rect);
  }
}
