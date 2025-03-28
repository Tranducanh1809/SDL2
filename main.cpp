#include <iostream>
#include <SDL.h>
#include <SDL_image.h>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <ctime>

using namespace std;

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
const int TILE_SIZE = 40;
const int MAP_WIDTH = SCREEN_WIDTH / TILE_SIZE;
const int MAP_HEIGHT = SCREEN_HEIGHT / TILE_SIZE;
const float BULLET_SPEED = 0.5;

class Wall {
public:
    int x, y;
    SDL_Rect rect;
    SDL_Texture* texture;

    Wall(int startX, int startY, SDL_Texture* tex) : x(startX), y(startY), texture(tex) {
        rect = {x, y, TILE_SIZE, TILE_SIZE};
    }

    void render(SDL_Renderer* renderer) {
        if (texture) {
            SDL_RenderCopy(renderer, texture, nullptr, &rect);
        }
    }
};

class Bullet {
public:
    float x, y, dx, dy;
    SDL_Rect rect;
    bool active;
    SDL_Renderer* renderer;

    Bullet(int startX, int startY, int dirX, int dirY, SDL_Renderer* ren)
        : renderer(ren), active(true) {
        x = startX - 5; // Center bullet
        y = startY - 5;
        dx = dirX;
        dy = dirY;
        rect = {x, y, 10, 10};
    }

    void update() {
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

    void render() {
        if (active) {
            SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
            SDL_RenderFillRect(renderer, &rect);
        }
    }
};

class PlayTank {
public:
    int x, y;
    SDL_Rect rect;
    SDL_Texture* textureUp, *textureDown, *textureLeft, *textureRight, *currentTexture;
    SDL_Renderer* renderer;
    vector<Wall>* walls;
    vector<Bullet>* bullets;
    int dirX, dirY;

    PlayTank(int startX, int startY, SDL_Renderer* ren, vector<Wall>* wallList, vector<Bullet>* bulletList)
        : x(startX), y(startY), renderer(ren), walls(wallList), bullets(bulletList), dirX(0), dirY(-1) {
        rect = {x, y, TILE_SIZE, TILE_SIZE};
        textureUp = IMG_LoadTexture(renderer, "yellow_up.png");
        textureDown = IMG_LoadTexture(renderer, "yellow_down.png");
        textureLeft = IMG_LoadTexture(renderer, "yellow_left.png");
        textureRight = IMG_LoadTexture(renderer, "yellow_right.png");
        currentTexture = textureUp;
    }

    bool checkCollision(int newX, int newY) {
        SDL_Rect newRect = {newX, newY, TILE_SIZE, TILE_SIZE};
        for (auto& wall : *walls) {
            if (SDL_HasIntersection(&newRect, &wall.rect)) return true;
        }
        return false;
    }

    void handleEvent(SDL_Event& e) {
        if (e.type == SDL_KEYDOWN) {
            int newX = x, newY = y;
            switch (e.key.keysym.sym) {
                case SDLK_UP:    newY -= TILE_SIZE; currentTexture = textureUp; dirX = 0; dirY = -1; break;
                case SDLK_DOWN:  newY += TILE_SIZE; currentTexture = textureDown; dirX = 0; dirY = 1; break;
                case SDLK_LEFT:  newX -= TILE_SIZE; currentTexture = textureLeft; dirX = -1; dirY = 0; break;
                case SDLK_RIGHT: newX += TILE_SIZE; currentTexture = textureRight; dirX = 1; dirY = 0; break;
                case SDLK_SPACE: bullets->emplace_back(x + TILE_SIZE / 2, y + TILE_SIZE / 2, dirX, dirY, renderer); break;
            }
            if (!checkCollision(newX, newY) && newX >= 0 && newY >= 0 && newX + TILE_SIZE <= SCREEN_WIDTH && newY + TILE_SIZE <= SCREEN_HEIGHT) {
                x = newX;
                y = newY;
                rect.x = x;
                rect.y = y;
            }
        }
    }

    void render() {
        if (currentTexture) {
            SDL_RenderCopy(renderer, currentTexture, nullptr, &rect);
        }
    }
};

class EnemyTank {
public:
    int x, y;
    int dirX, dirY;
    int moveDelay, shootDelay;
    SDL_Rect rect;
    bool active;
    vector<Bullet> bullets;
    SDL_Renderer* renderer;
    SDL_Texture* textureUp, *textureDown, *textureLeft, *textureRight, *currentTexture;

    // Constructor của EnemyTank
    EnemyTank(int startX, int startY, SDL_Renderer* ren) : renderer(ren) {
        moveDelay = 12; // Tốc độ di chuyển
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

    // Hàm di chuyển xe tăng địch
    void move(const vector<Wall>& walls, const vector<EnemyTank>& enemies) {
        if (--moveDelay > 0) return;
        moveDelay = 3000;

        int newX = x;
        int newY = y;
        SDL_Rect newRect;

        int randomDirection = rand() % 4;
        switch (randomDirection) {
            case 0: newY -= TILE_SIZE; dirX = 0; dirY = -1; currentTexture = textureUp; break;
            case 1: newY += TILE_SIZE; dirX = 0; dirY = 1; currentTexture = textureDown; break;
            case 2: newX -= TILE_SIZE; dirX = -1; dirY = 0; currentTexture = textureLeft; break;
            case 3: newX += TILE_SIZE; dirX = 1; dirY = 0; currentTexture = textureRight; break;
        }

        newRect = {newX, newY, TILE_SIZE, TILE_SIZE};

        // Kiểm tra va chạm với tường
        for (const auto& wall : walls) {
            if (SDL_HasIntersection(&newRect, &wall.rect)) {
                return;
            }
        }

        // Kiểm tra va chạm với xe tăng địch khác
        for (const auto& enemy : enemies) {
            if (this != &enemy && SDL_HasIntersection(&newRect, &enemy.rect)) {
                return;
            }
        }

        // Kiểm tra ranh giới màn hình
        if (newX >= 0 && newX < SCREEN_WIDTH - TILE_SIZE && newY >= 0 && newY < SCREEN_HEIGHT - TILE_SIZE) {
            x = newX;
            y = newY;
            rect.x = x;
            rect.y = y;
        }
    }

    // Hàm bắn đạn
    void shoot() {
        if (--shootDelay > 0) return;
        shootDelay = 5000; // Reset delay
        bullets.emplace_back(x + TILE_SIZE / 2, y + TILE_SIZE / 2, dirX, dirY, renderer);
    }

    // Hàm cập nhật đạn
 void updateBullets(vector<Wall>& walls) {
    for (auto& bullet : bullets) {
        bullet.x += bullet.dx * 0.1; // Giảm tốc độ xuống 30% so với đạn người chơi
        bullet.y += bullet.dy * 0.1;
        bullet.rect.x = bullet.x;
        bullet.rect.y = bullet.y;
    }

    bullets.erase(remove_if(bullets.begin(), bullets.end(), [&](Bullet& b) {
        for (auto it = walls.begin(); it != walls.end(); ++it) {
            if (SDL_HasIntersection(&b.rect, &it->rect)) {
                walls.erase(it);
                return true;
            }
        }
        return !b.active;
    }), bullets.end());
}

    // Hàm vẽ xe tăng địch
    void render() {
        if (currentTexture) {
            SDL_RenderCopy(renderer, currentTexture, nullptr, &rect);
        }
        for (auto& bullet : bullets) {
            bullet.render();
        }
    }
};

class Game {
public:
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* wallTexture;
    bool running;
    vector<Wall> walls;
    vector<Bullet> bullets;
    PlayTank* player;
    vector<EnemyTank> enemies;

    Game() {
        running = true;
        SDL_Init(SDL_INIT_VIDEO);
        window = SDL_CreateWindow("Battle City", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
        wallTexture = IMG_LoadTexture(renderer, "red_brick.png");

        generateWalls();
        player = new PlayTank(SCREEN_WIDTH / 2, SCREEN_HEIGHT - TILE_SIZE * 2, renderer, &walls, &bullets);

        srand(time(0)); // Khởi tạo seed cho rand()
        generateRandomEnemies();
    }

  void generateWalls() {
    // Tạo viền ngoài bản đồ
    for (int i = 0; i < MAP_HEIGHT; i++) {
        for (int j = 0; j < MAP_WIDTH; j++) {
            if (i == 0 || i == MAP_HEIGHT - 1 || j == 0 || j == MAP_WIDTH - 1) {
                walls.emplace_back(j * TILE_SIZE, i * TILE_SIZE, wallTexture);
            }
        }
    }

    // Bản đồ mẫu chữ "BC" lớn hơn (7x14 ô)
    string mapLayout[7] = {
        "BBBBB    CCCCC ",
        "B    B   C     ",
        "B    B   C     ",
        "BBBBB    C     ",
        "B    B   C     ",
        "B    B   C     ",
        "BBBBB    CCCCC "
    };

    int startX = MAP_WIDTH / 2 - 7; // Căn giữa chữ "BC"
    int startY = MAP_HEIGHT / 2 - 3;

    // Duyệt qua lưới để đặt tường theo chữ
    for (int i = 0; i < 7; i++) {
        for (int j = 0; j < 14; j++) {
            if (mapLayout[i][j] != ' ') {
                walls.emplace_back((startX + j) * TILE_SIZE, (startY + i) * TILE_SIZE, wallTexture);
            }
        }
    }
}

  void generateRandomEnemies() {
        for (int i = 0; i < 3; ++i) {
            bool validPosition = false;
            int randomX, randomY;

            while (!validPosition) {
                randomX = (rand() % (MAP_WIDTH - 2)) * TILE_SIZE + TILE_SIZE;
                randomY = (rand() % (MAP_HEIGHT - 2)) * TILE_SIZE + TILE_SIZE;

                SDL_Rect enemyRect = {randomX, randomY, TILE_SIZE, TILE_SIZE};
                validPosition = true;

                // Kiểm tra va chạm với tường và các xe tăng địch khác
                for (const auto& wall : walls) {
                    if (SDL_HasIntersection(&enemyRect, &wall.rect)) {
                        validPosition = false;
                        break;
                    }
                }

                for (const auto& enemy : enemies) {
                    if (SDL_HasIntersection(&enemyRect, &enemy.rect)) {
                        validPosition = false;
                        break;
                    }
                }

                // Kiểm tra va chạm với player
                if (SDL_HasIntersection(&enemyRect, &player->rect)) {
                    validPosition = false;
                    break;
                }
            }
            enemies.emplace_back(randomX, randomY, renderer);
        }
    }

updateBullets() {
    for (auto& bullet : bullets) {
        bullet.update();
    }

    bullets.erase(remove_if(bullets.begin(), bullets.end(), [&](Bullet& b) {
        for (auto it = walls.begin(); it != walls.end(); ++it) {
            if (SDL_HasIntersection(&b.rect, &it->rect)) {
                walls.erase(it);
                return true;
            }
        }

        // Kiểm tra va chạm với xe tăng địch
        for (auto enemyIt = enemies.begin(); enemyIt != enemies.end(); ++enemyIt) {
            if (SDL_HasIntersection(&b.rect, &enemyIt->rect)) {
                enemies.erase(enemyIt);
                return true;
            }
        }
        return !b.active;
    }), bullets.end());

    // Kiểm tra nếu tiêu diệt hết địch
    if (enemies.empty()) {
          running = false; // Kết thúc game
    }
}

void updateEnemyBullets() {
    for (auto& enemy : enemies) {
        enemy.updateBullets(walls);
        for (auto& bullet : enemy.bullets) {
            if (SDL_HasIntersection(&bullet.rect, &player->rect)) {
                running = false; // Kết thúc game
                return;
            }
        }
    }
}

 void updateEnemies() {
        for (auto& enemy : enemies) {
            enemy.move(walls, enemies); // Truyền danh sách enemies
            enemy.shoot();

            if (SDL_HasIntersection(&enemy.rect, &player->rect)) {

                running = false;
                return;
            }
        }
    }
    void run() {
        SDL_Event e;
        while (running) {
            while (SDL_PollEvent(&e)) {
                if (e.type == SDL_QUIT) running = false;
                player->handleEvent(e);
            }
            updateBullets();
            updateEnemyBullets(); // Cập nhật đạn địch
            updateEnemies(); // Cập nhật vị trí địch

            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);
            for (auto& wall : walls) wall.render(renderer);
            for (auto& bullet : bullets) bullet.render();
            player->render();
            for (auto& enemy : enemies) enemy.render(); // Vẽ địch

            SDL_RenderPresent(renderer);
        }
    }
};

int main(int argc, char* argv[]) {
    Game game;
    if (game.running) game.run();
    return 0;
}
