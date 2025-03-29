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
   int type; // 0: red_brick, 1: white_brick, 2: water

    Wall(int startX, int startY, SDL_Texture* tex, int wallType = 0)
        : x(startX), y(startY), texture(tex), type(wallType) {
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
    SDL_Texture* textureUp, *textureDown, *textureLeft, *textureRight, *currentTexture; // Textures for 4 directions

    Bullet(int startX, int startY, int dirX, int dirY, SDL_Renderer* ren)
        : renderer(ren), active(true) {
        x = startX - 5; // Center bullet
        y = startY - 5;
        dx = dirX;
        dy = dirY;
        rect = {x, y, 20, 20};

        // Load the bullet images
        textureUp = IMG_LoadTexture(renderer, "bullet_up.png"); // Replace with your image file names
        textureDown = IMG_LoadTexture(renderer, "bullet_down.png");
        textureLeft = IMG_LoadTexture(renderer, "bullet_left.png");
        textureRight = IMG_LoadTexture(renderer, "bullet_right.png");

        if (!textureUp || !textureDown || !textureLeft || !textureRight) {
            cout << "Failed to load bullet textures: " << IMG_GetError() << endl;
        }

        // Set the current texture based on the direction
        if (dirY == -1) currentTexture = textureUp;
        else if (dirY == 1) currentTexture = textureDown;
        else if (dirX == -1) currentTexture = textureLeft;
        else if (dirX == 1) currentTexture = textureRight;
    }

    ~Bullet() {
        if (textureUp) SDL_DestroyTexture(textureUp);
        if (textureDown) SDL_DestroyTexture(textureDown);
        if (textureLeft) SDL_DestroyTexture(textureLeft);
        if (textureRight) SDL_DestroyTexture(textureRight);
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
        if (active && currentTexture) {
            SDL_RenderCopy(renderer, currentTexture, nullptr, &rect);
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

 checkCollision(int newX, int newY, const vector<Wall>& walls) {
    SDL_Rect newRect = {newX, newY, TILE_SIZE, TILE_SIZE};
    for (const auto& wall : walls) {
        if (SDL_HasIntersection(&newRect, &wall.rect)) return true; // Kiểm tra va chạm với tất cả các loại tường
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
            case SDLK_SPACE: bullets->emplace_back(x + TILE_SIZE / 2 - 8, y + TILE_SIZE / 2 - 8, dirX, dirY, renderer); break; // Sửa vị trí bắn
        }
        if (!checkCollision(newX, newY, *walls) && newX >= 0 && newY >= 0 && newX + TILE_SIZE <= SCREEN_WIDTH && newY + TILE_SIZE <= SCREEN_HEIGHT) {
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
bool checkCollision(int newX, int newY, const vector<Wall>& walls, const vector<EnemyTank>& enemies) {
    SDL_Rect newRect = {newX, newY, TILE_SIZE, TILE_SIZE};

    for (const auto& wall : walls) {
        if (SDL_HasIntersection(&newRect, &wall.rect)) { // Kiểm tra va chạm với tất cả các loại tường
            return true;
        }
    }

    for (const auto& enemy : enemies) {
        if (this != &enemy && SDL_HasIntersection(&newRect, &enemy.rect)) {
            return true;
        }
    }

    return false;
}
 void move(const vector<Wall>& walls, const vector<EnemyTank>& enemies) {
    if (--moveDelay > 0) return;
    moveDelay = 3000;

    int newX = x;
    int newY = y;

    int randomDirection = rand() % 4;
    switch (randomDirection) {
        case 0: newY -= TILE_SIZE; dirX = 0; dirY = -1; currentTexture = textureUp; break;
        case 1: newY += TILE_SIZE; dirX = 0; dirY = 1; currentTexture = textureDown; break;
        case 2: newX -= TILE_SIZE; dirX = -1; dirY = 0; currentTexture = textureLeft; break;
        case 3: newX += TILE_SIZE; dirX = 1; dirY = 0; currentTexture = textureRight; break;
    }

    // Sử dụng hàm checkCollision() để kiểm tra va chạm
    if (!checkCollision(newX, newY, walls, enemies) && newX >= 0 && newX < SCREEN_WIDTH - TILE_SIZE && newY >= 0 && newY < SCREEN_HEIGHT - TILE_SIZE) {
        x = newX;
        y = newY;
        rect.x = x;
        rect.y = y;
    }
}

  void shoot() {
    if (--shootDelay > 0) return;
    shootDelay = 5000; // Reset delay
    bullets.emplace_back(x + TILE_SIZE / 2 - 8, y + TILE_SIZE / 2 - 8, dirX, dirY, renderer); // Sửa vị trí bắn
}
    // Hàm cập nhật đạn
 void updateBullets(vector<Wall>& walls, vector<EnemyTank>& enemies) {
    vector<int> bulletsToRemove;

    for (int i = 0; i < bullets.size(); ++i) {
        Bullet& b = bullets[i];
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
            for (auto& enemy : enemies) {
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

    sort(bulletsToRemove.rbegin(), bulletsToRemove.rend());
    for (int index : bulletsToRemove) {
        if (index >= 0 && index < bullets.size()) {
            bullets.erase(bullets.begin() + index);
        }
    }

    for (auto& bullet : bullets) {
        bullet.x += bullet.dx * 0.1;
        bullet.y += bullet.dy * 0.1;
        bullet.rect.x = bullet.x;
        bullet.rect.y = bullet.y;
    }
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
    SDL_Texture* whiteBrickTexture;
    SDL_Texture* waterTexture;
    SDL_Texture* gameOverTexture;
    SDL_Texture* youWinTexture;
    SDL_Texture* menuTexture;
    bool running;
    vector<Wall> walls;
    vector<Bullet> bullets;
    PlayTank* player;
    vector<EnemyTank> enemies;
    enum GameState { PLAYING, WIN, LOSE, MENU };
    GameState gameState;
    Uint32 gameOverStartTime;
    Uint32 winStartTime;

    Game() {
        running = true;
        SDL_Init(SDL_INIT_VIDEO);
        window = SDL_CreateWindow("Battle City", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
        wallTexture = IMG_LoadTexture(renderer, "red_brick.png");
        whiteBrickTexture = IMG_LoadTexture(renderer, "white_brick.png");
        waterTexture = IMG_LoadTexture(renderer, "water.png");
        gameOverTexture = IMG_LoadTexture(renderer, "game_over.png");
        youWinTexture = IMG_LoadTexture(renderer, "you_win.png");
        menuTexture = IMG_LoadTexture(renderer, "menu.png");
        gameState = MENU;
        srand(time(0));
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

        "CCBBBW   WCCBBC ",
        "B    B   BB    ",
        "B    B   C    ",
        "WBCCB    W    ",
        "B    B   C    ",
        "B   WB   CBW   ",
        "CCBBBW   BBCCBB "
    };

    int startX = MAP_WIDTH / 2 - 7;
    int startY = MAP_HEIGHT / 2 - 3;

    for (int i = 0; i < 7; i++) {
        for (int j = 0; j < 14; j++) {
           if (mapLayout[i][j] == 'B') {
                walls.emplace_back((startX + j) * TILE_SIZE, (startY + i) * TILE_SIZE, wallTexture);
            } else if (mapLayout[i][j] == 'C') {
                walls.emplace_back((startX + j) * TILE_SIZE, (startY + i) * TILE_SIZE, whiteBrickTexture, 1);
            } else if (mapLayout[i][j] == 'W') { // Thêm nước
                walls.emplace_back((startX + j) * TILE_SIZE, (startY + i) * TILE_SIZE, waterTexture, 2);
            }
        }
    }
}

  void generateRandomEnemies() {
        for (int i = 0; i < 4; ++i) {
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

void updateBullets() {
    for (auto& bullet : bullets) {
        bullet.update();
    }

    // Kiểm tra va chạm đạn với tường và địch
    bullets.erase(remove_if(bullets.begin(), bullets.end(), [&](Bullet& b) {
        for (auto it = walls.begin(); it != walls.end(); ++it) {
            if (it->type == 0 && SDL_HasIntersection(&b.rect, &it->rect)) {
                walls.erase(it);
                return true;
            } else if (it->type == 1 && SDL_HasIntersection(&b.rect, &it->rect)) {
                return true;
            }
        }
        for (int j = 0; j < enemies.size(); ++j) {
            EnemyTank& enemy = enemies[j];
            if (SDL_HasIntersection(&b.rect, &enemy.rect)) {
                enemies.erase(enemies.begin() + j);
                return true;
            }
        }
        if (b.rect.x < 0 || b.rect.x > SCREEN_WIDTH || b.rect.y < 0 || b.rect.y > SCREEN_HEIGHT) {
            return true;
        }
        return !b.active;
    }), bullets.end());

  if (enemies.empty() && gameState == PLAYING) {
        gameState = WIN; // Cập nhật trạng thái khi thắng
        running = false; // Kết thúc vòng lặp
    }
}

void updateEnemyBullets() {
    for (auto& enemy : enemies) {
        enemy.updateBullets(walls, enemies);
        for (auto& bullet : enemy.bullets) {
            if (SDL_HasIntersection(&bullet.rect, &player->rect)) {
                gameState = LOSE; // Cập nhật trạng thái khi thua
                running = false; // Kết thúc vòng lặp
                return;
            }
        }
    }
}

 void updateEnemies() {
    for (auto& enemy : enemies) {
        enemy.move(walls, enemies);
        enemy.shoot();

        if (SDL_HasIntersection(&enemy.rect, &player->rect)) {
            gameState = LOSE; // Cập nhật trạng thái khi thua
            running = false; // Kết thúc vòng lặp
            return;
        }
    }
}
void renderGameOver() {
    if (gameOverTexture) {
        SDL_Rect gameOverRect = {SCREEN_WIDTH / 2 - 200, SCREEN_HEIGHT / 2 - 100, 400, 200}; // Điều chỉnh vị trí và kích thước
        SDL_RenderCopy(renderer, gameOverTexture, nullptr, &gameOverRect);
    }
}
void renderYouWin() {
    if (youWinTexture) {
        SDL_Rect youWinRect = {SCREEN_WIDTH / 2 - 200, SCREEN_HEIGHT / 2 - 100, 400, 200}; // Điều chỉnh vị trí và kích thước
        SDL_RenderCopy(renderer, youWinTexture, nullptr, &youWinRect);
    }
}
void renderMenu() {
        if (menuTexture) {
            SDL_RenderCopy(renderer, menuTexture, nullptr, nullptr);
        }
    }
void handleMenuEvents(SDL_Event& e) {
        if (e.type == SDL_KEYDOWN) {
            switch (e.key.keysym.sym) {
                case SDLK_RETURN: // Thay đổi từ SDLK_SPACE thành SDLK_RETURN
                    gameState = PLAYING;
                    running = true;
                    generateWalls();
                    player = new PlayTank(SCREEN_WIDTH / 2, SCREEN_HEIGHT - TILE_SIZE * 2, renderer, &walls, &bullets);
                    srand(time(0));
                    generateRandomEnemies();
                    break;
                case SDLK_ESCAPE:
                    running = false;
                    break;
                default:
                    break;
            }
        }
    }

 void run() {
        SDL_Event e;
        while (running) {
            while (SDL_PollEvent(&e)) {
                if (e.type == SDL_QUIT) {
                    running = false;
                    return;
                }

                if (e.type == SDL_KEYDOWN) { // Thêm xử lý phím ESC
                    if (e.key.keysym.sym == SDLK_ESCAPE) {
                        running = false;
                        return;
                    }
                }

                if (gameState == MENU) {
                    handleMenuEvents(e);
                } else {
                    player->handleEvent(e);
                }
            }

            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);

            if (gameState == MENU) {
                renderMenu();
            } else if (gameState == PLAYING) {
                updateBullets();
                updateEnemyBullets();
                updateEnemies();

                for (auto& wall : walls) wall.render(renderer);

                for (auto& bullet : bullets) bullet.render();
                for (auto& enemy : enemies) {
                    for (auto& bullet : enemy.bullets) {
                        bullet.render();
                    }
                }

                player->render();
                for (auto& enemy : enemies) enemy.render();
            } else if (gameState == WIN) {
                renderYouWin();
                if (winStartTime == 0) {
                    winStartTime = SDL_GetTicks();
                }
                if (SDL_GetTicks() - winStartTime > 1000) {
                    gameState = MENU;
                    running = false;
                }
            } else if (gameState == LOSE) {
                renderGameOver();
                if (gameOverStartTime == 0) {
                    gameOverStartTime = SDL_GetTicks();
                }
                if (SDL_GetTicks() - gameOverStartTime > 1000) {
                    gameState = MENU;
                    running = false;
                }
            }

            SDL_RenderPresent(renderer);
        }

        if (gameState != MENU) {
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);

            if (gameState == WIN) {
                renderYouWin();
            } else if (gameState == LOSE) {
                renderGameOver();
            }

            SDL_RenderPresent(renderer);
            SDL_Delay(1000);
            gameState = MENU;
            running = true;
            walls.clear();
            bullets.clear();
            enemies.clear();
            delete player;
            player = nullptr;
            winStartTime = 0;
            gameOverStartTime = 0;
            run();
        }
    }
};

int main(int argc, char* argv[]) {
    Game game;
    if (game.running) game.run();
    return 0;
}
