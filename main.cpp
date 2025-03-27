#include <iostream>
#include <SDL.h>
#include <SDL_image.h>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <algorithm>

using namespace std;

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
const int TILE_SIZE = 40;
const int MAP_WIDTH = SCREEN_WIDTH / TILE_SIZE;
const int MAP_HEIGHT = SCREEN_HEIGHT / TILE_SIZE;
const int BULLET_SPEED = 2;
const int PLAYER_SPEED = 4;
const int ENEMY_SPEED = 2;

class Wall {
public:
    int x, y;
    SDL_Rect rect;
    bool active;
    SDL_Texture* texture;

    Wall(int startX, int startY, SDL_Texture* tex) : x(startX), y(startY), active(true), texture(tex) {
        rect = {x, y, TILE_SIZE, TILE_SIZE};
    }

    void render(SDL_Renderer* renderer) {
        if (active && texture) {
            SDL_RenderCopy(renderer, texture, nullptr, &rect);
        }
    }
};

class PlayTank {
public:
    int x, y;
    SDL_Rect rect;
    vector<Wall>* walls;

    PlayTank(int startX, int startY, vector<Wall>* wallList) : x(startX), y(startY), walls(wallList) {
        rect = {x, y, TILE_SIZE, TILE_SIZE};
    }

    bool checkCollision(int newX, int newY) {
        SDL_Rect newRect = {newX, newY, TILE_SIZE, TILE_SIZE};
        for (auto& wall : *walls) {
            if (SDL_HasIntersection(&newRect, &wall.rect)) {
                return true;
            }
        }
        return false;
    }

    void handleEvent(SDL_Event& e) {
        if (e.type == SDL_KEYDOWN) {
            int newX = x, newY = y;
            switch (e.key.keysym.sym) {
                case SDLK_UP: newY -= PLAYER_SPEED; break;
                case SDLK_DOWN: newY += PLAYER_SPEED; break;
                case SDLK_LEFT: newX -= PLAYER_SPEED; break;
                case SDLK_RIGHT: newX += PLAYER_SPEED; break;
            }
            if (!checkCollision(newX, newY) && newX >= 0 && newY >= 0 && newX + TILE_SIZE <= SCREEN_WIDTH && newY + TILE_SIZE <= SCREEN_HEIGHT) {
                x = newX;
                y = newY;
                rect.x = x;
                rect.y = y;
            }
        }
    }

    void render(SDL_Renderer* renderer) {
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
        SDL_RenderFillRect(renderer, &rect);
    }
};

class EnemyTank {
public:
    int x, y;
    SDL_Rect rect;
    vector<Wall>* walls;
    int moveDelay;

    EnemyTank(int startX, int startY, vector<Wall>* wallList) : x(startX), y(startY), walls(wallList), moveDelay(30) {
        rect = {x, y, TILE_SIZE, TILE_SIZE};
    }

    bool checkCollision(int newX, int newY) {
        SDL_Rect newRect = {newX, newY, TILE_SIZE, TILE_SIZE};
        for (auto& wall : *walls) {
            if (SDL_HasIntersection(&newRect, &wall.rect)) {
                return true;
            }
        }
        return false;
    }

    void move() {
        if (--moveDelay > 0) return;
        moveDelay = 30;
        int r = rand() % 4;
        int newX = x, newY = y;

        switch (r) {
            case 0: newY -= TILE_SIZE; break;
            case 1: newY += TILE_SIZE; break;
            case 2: newX -= TILE_SIZE; break;
            case 3: newX += TILE_SIZE; break;
        }

        if (!checkCollision(newX, newY) && newX >= 0 && newY >= 0 && newX + TILE_SIZE <= SCREEN_WIDTH && newY + TILE_SIZE <= SCREEN_HEIGHT) {
            x = newX;
            y = newY;
            rect.x = x;
            rect.y = y;
        }
    }

    void render(SDL_Renderer* renderer) {
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_RenderFillRect(renderer, &rect);
    }
};

class Game {
public:
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* wallTexture;
    bool running;
    vector<Wall> walls;
    PlayTank* player;
    vector<EnemyTank> enemies;

    Game() {
        running = true;

        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
            cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << endl;
            running = false;
            return;
        }

        window = SDL_CreateWindow("Battle City", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
        if (!window) {
            cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << endl;
            running = false;
            return;
        }

        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
        if (!renderer) {
            cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << endl;
            running = false;
            return;
        }

        wallTexture = IMG_LoadTexture(renderer, "red_brick.png");
        if (!wallTexture) {
            cerr << "Failed to load wall texture! SDL_Error: " << SDL_GetError() << endl;
            running = false;
            return;
        }

        generateWalls();
        player = new PlayTank(SCREEN_WIDTH / 2, SCREEN_HEIGHT - TILE_SIZE * 2, &walls);
        generateEnemies();
    }

    void generateWalls() {
        for (int i = 1; i < MAP_HEIGHT - 1; i++) {
            for (int j = 1; j < MAP_WIDTH - 1; j++) {
                if (rand() % 5 == 0) {
                    walls.emplace_back(j * TILE_SIZE, i * TILE_SIZE, wallTexture);
                }
            }
        }
    }

    void generateEnemies() {
        for (int i = 0; i < 3; i++) {
            enemies.emplace_back(rand() % SCREEN_WIDTH, rand() % SCREEN_HEIGHT, &walls);
        }
    }

    void update() {
        for (auto& enemy : enemies) {
            enemy.move();
        }
    }

    void run() {
        SDL_Event e;
        while (running) {
            while (SDL_PollEvent(&e)) {
                if (e.type == SDL_QUIT) running = false;
                player->handleEvent(e);
            }
            update();
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);
            for (auto& wall : walls) wall.render(renderer);
            player->render(renderer);
            for (auto& enemy : enemies) enemy.render(renderer);
            SDL_RenderPresent(renderer);
        }
    }
};

int main(int argc, char* argv[]) {
    Game game;
    if (game.running) game.run();
    return 0;
}
