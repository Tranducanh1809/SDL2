#include <iostream>
#include <SDL.h>
#include <vector>
#include <cstdlib> // Cho rand()
#include <ctime>   // Cho time()
using namespace std;

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
const int TILE_SIZE = 40;
const int MAP_WIDTH = SCREEN_WIDTH / TILE_SIZE;
const int MAP_HEIGHT = SCREEN_HEIGHT / TILE_SIZE;

class Wall {
public:
    int x, y;
    SDL_Rect rect;
    bool active;

    Wall(int startX, int startY) : x(startX), y(startY), active(true) {
        rect = {x, y, TILE_SIZE, TILE_SIZE};
    }

    void render(SDL_Renderer* renderer) {
        if (active) {
            SDL_SetRenderDrawColor(renderer, 150, 75, 0, 255);
            SDL_RenderFillRect(renderer, &rect);
        }
    }
};

class PlayerTank {
public:
    int x, y;
    SDL_Rect rect;

    PlayerTank(int startX, int startY) : x(startX), y(startY) {
        rect = {x, y, TILE_SIZE, TILE_SIZE};
    }

    void move(int dx, int dy, const vector<Wall>& walls) {
        int newX = x + dx;
        int newY = y + dy;

        SDL_Rect newRect = {newX, newY, TILE_SIZE, TILE_SIZE};

        // Kiểm tra va chạm với tường
        for (const auto& wall : walls) {
            if (wall.active && SDL_HasIntersection(&newRect, &wall.rect)) {
                return;  // Không di chuyển nếu va chạm
            }
        }

        // Kiểm tra giới hạn màn hình
        if (newX >= 0 && newX <= SCREEN_WIDTH - TILE_SIZE &&
            newY >= 0 && newY <= SCREEN_HEIGHT - TILE_SIZE) {
            x = newX;
            y = newY;
            rect.x = x;
            rect.y = y;
        }
    }

    void render(SDL_Renderer* renderer) {
        SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
        SDL_RenderFillRect(renderer, &rect);
    }
};

class Game {
public:
    SDL_Window* window;
    SDL_Renderer* renderer;
    bool running;
    vector<Wall> walls;
    PlayerTank player;

    Game() : player(((MAP_WIDTH - 1) / 2) * TILE_SIZE, (MAP_HEIGHT - 2) * TILE_SIZE) {
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
        srand(time(0)); // Khởi tạo bộ sinh số ngẫu nhiên
        generateWalls();
    }

    void generateWalls() {
        for (int i = 1; i < MAP_HEIGHT - 1; i++) {
            for (int j = 1; j < MAP_WIDTH - 1; j++) {
                if (rand() % 7 == 0) { // Điều chỉnh tỷ lệ xuất hiện tường (1/5)
                    walls.emplace_back(j * TILE_SIZE, i * TILE_SIZE);
                }
            }
        }
    }

    void render() {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // Vẽ các bức tường
        for (auto& wall : walls) {
            wall.render(renderer);
        }

        // Vẽ xe tăng người chơi
        player.render(renderer);
        SDL_RenderPresent(renderer);
    }

    void handleEvents() {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            } else if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                    case SDLK_UP: player.move(0, -TILE_SIZE, walls); break;
                    case SDLK_DOWN: player.move(0, TILE_SIZE, walls); break;
                    case SDLK_LEFT: player.move(-TILE_SIZE, 0, walls); break;
                    case SDLK_RIGHT: player.move(TILE_SIZE, 0, walls); break;
                }
            }
        }
    }

    void run() {
        while (running) {
            handleEvents();
            render();
            SDL_Delay(16);
        }
    }

    ~Game() {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
    }
};

int main(int argc, char* argv[]) {
    Game game;  // Khởi tạo game
    if (game.running) {
        game.run();  // Chạy game nếu game vẫn đang chạy
    }
    return 0;
}
