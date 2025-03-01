#include <SDL.h>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>

// Cấu trúc lưu trữ thông tin obstacle
struct Obstacle {
    SDL_Rect rect;
    float speed;
};

int main(int argc, char* argv[]) {
    // Khởi tạo SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cout << "SDL không khởi tạo được! Lỗi: " << SDL_GetError() << std::endl;
        return 1;
    }

    // Tạo cửa sổ game
    SDL_Window* window = SDL_CreateWindow("Dino Game SDL2",
                                          SDL_WINDOWPOS_CENTERED,
                                          SDL_WINDOWPOS_CENTERED,
                                          800, 200,
                                          SDL_WINDOW_SHOWN);
    if (window == nullptr) {
        std::cout << "Không tạo được cửa sổ! Lỗi: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    // Tạo renderer
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == nullptr) {
        std::cout << "Không tạo được renderer! Lỗi: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Thiết lập thông số Dino (hình chữ nhật)
    SDL_Rect dino;
    dino.x = 50;
    dino.w = 40;
    dino.h = 40;
    // Đặt Dino ở dưới cùng của cửa sổ (chiều cao cửa sổ 200)
    dino.y = 200 - dino.h;

    float dinoVelocityY = 0.0f;
    const float gravity = 0.8f;
    const float jumpForce = -15.0f;
    bool isJumping = false;

    // Mảng chứa các obstacle
    std::vector<Obstacle> obstacles;
    Uint32 lastObstacleTime = SDL_GetTicks();
    const Uint32 obstacleInterval = 1500; // tạo obstacle mỗi 1.5 giây

    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    bool quit = false;
    bool gameOver = false;
    SDL_Event e;

    Uint32 lastTime = SDL_GetTicks();

    // Vòng lặp game chính
    while (!quit) {
        // Xử lý sự kiện
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT)
                quit = true;

            if (e.type == SDL_KEYDOWN) {
                // Nhấn phím Space để nhảy (chỉ cho phép nhảy khi chưa nhảy)
                if (e.key.keysym.sym == SDLK_SPACE && !isJumping && !gameOver) {
                    dinoVelocityY = jumpForce;
                    isJumping = true;
                }
            }
        }

        // Tính delta time (thời gian giữa các frame)
        Uint32 currentTime = SDL_GetTicks();
        float deltaTime = (currentTime - lastTime) / 1000.0f; // chuyển từ ms sang giây
        lastTime = currentTime;

        // Cập nhật vị trí của Dino và áp dụng trọng lực
        dino.y += static_cast<int>(dinoVelocityY);
        dinoVelocityY += gravity;
        if (dino.y >= 200 - dino.h) {
            dino.y = 200 - dino.h;
            dinoVelocityY = 0;
            isJumping = false;
        }

        // Tạo obstacle mới sau mỗi obstacleInterval
        if (currentTime - lastObstacleTime > obstacleInterval) {
            Obstacle obs;
            // Kích thước ngẫu nhiên từ 20 đến 40
            int size = rand() % 21 + 20;
            obs.rect.w = size;
            obs.rect.h = size;
            obs.rect.x = 800;           // bắt đầu từ bên phải cửa sổ
            obs.rect.y = 200 - size;      // đặt obstacle ở dưới cùng
            obs.speed = 200.0f;           // tốc độ di chuyển (pixel/giây)
            obstacles.push_back(obs);
            lastObstacleTime = currentTime;
        }

        // Cập nhật vị trí của các obstacle và kiểm tra va chạm
        for (auto it = obstacles.begin(); it != obstacles.end();) {
            it->rect.x -= static_cast<int>(it->speed * deltaTime);
            // Xóa obstacle nếu nó ra khỏi màn hình
            if (it->rect.x + it->rect.w < 0) {
                it = obstacles.erase(it);
            } else {
                // Kiểm tra va chạm giữa Dino và obstacle
                if (SDL_HasIntersection(&dino, &it->rect)) {
                    gameOver = true;
                }
                ++it;
            }
        }

        // Vẽ màn hình game
        SDL_SetRenderDrawColor(renderer, 247, 247, 247, 255);
        SDL_RenderClear(renderer);

        // Vẽ Dino (màu xám)
        SDL_SetRenderDrawColor(renderer, 85, 85, 85, 255);
        SDL_RenderFillRect(renderer, &dino);

        // Vẽ các obstacle (màu đỏ)
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        for (const auto &obs : obstacles) {
            SDL_RenderFillRect(renderer, &obs.rect);
        }

        SDL_RenderPresent(renderer);

        // Nếu game kết thúc, tạm dừng 3 giây rồi thoát
        if (gameOver) {
            SDL_Delay(3000);
            quit = true;
        }
    }

    // Giải phóng tài nguyên
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
     cout<<"hi";
    return 0;
}
