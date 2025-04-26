#include "Snake.h"
#include <cstdlib>
#include <ctime>
#include <cmath>

SnakeGame::SnakeGame()
    : dir(1), moveCounter(0), running(true)
{
    std::srand((unsigned)std::time(nullptr));
    Restart();
}

void SnakeGame::Restart() {
    snake.clear();
    float cx = S_FRAME + (S_COLS / 2) * S_CELL;
    float cy = S_FRAME + (S_ROWS / 2) * S_CELL;
    snake.push_back({ cx,cy });
    snake.push_back({ cx - S_CELL,cy });
    snake.push_back({ cx - 2 * S_CELL,cy });
    dir = 1;
    running = true;
    spawnFood();
}

void SnakeGame::spawnFood() {
    bool ok = false;
    while (!ok) {
        ok = true;
        foodPos.x = float(S_FRAME + (std::rand() % S_COLS) * S_CELL + S_CELL / 2);
        foodPos.y = float(S_FRAME + (std::rand() % S_ROWS) * S_CELL + S_CELL / 2);
        for (auto& seg : snake) {
            float sx = seg.x + S_CELL / 2, sy = seg.y + S_CELL / 2;
            if (sx == foodPos.x && sy == foodPos.y) { ok = false; break; }
        }
    }
}

void SnakeGame::HandleInput() {
    if (!running) return;
    int nd = dir;
    if (Input::IsKeyPressed(SDLK_UP))    nd = 0;
    if (Input::IsKeyPressed(SDLK_RIGHT)) nd = 1;
    if (Input::IsKeyPressed(SDLK_DOWN))  nd = 2;
    if (Input::IsKeyPressed(SDLK_LEFT))  nd = 3;
    bool opp = (dir == 0 && nd == 2) || (dir == 2 && nd == 0) || (dir == 1 && nd == 3) || (dir == 3 && nd == 1);
    if (!opp) dir = nd;
}

void SnakeGame::Update() {
    if (!running) return;
    if (++moveCounter < 370) return;
    moveCounter = 0;
    for (int i = int(snake.size()) - 1; i > 0; --i) snake[i] = snake[i - 1];
    switch (dir) {
    case 0: snake[0].y -= S_CELL; break;
    case 1: snake[0].x += S_CELL; break;
    case 2: snake[0].y += S_CELL; break;
    case 3: snake[0].x -= S_CELL; break;
    }
    if (snake[0].x < S_FRAME || snake[0].x >= S_FRAME + S_COLS * S_CELL ||
        snake[0].y < S_FRAME || snake[0].y >= S_FRAME + S_ROWS * S_CELL) {
        running = false; return;
    }
    for (size_t i = 1; i < snake.size(); ++i)
        if (snake[0].x == snake[i].x && snake[0].y == snake[i].y) {
            running = false; return;
        }
    float dx = (snake[0].x + S_CELL / 2) - foodPos.x;
    float dy = (snake[0].y + S_CELL / 2) - foodPos.y;
    float sumR = S_CELL / 2 + S_FOOD_R;
    if (dx * dx + dy * dy <= sumR * sumR) {
        snake.push_back(snake.back());
        spawnFood();
    }
}

void SnakeGame::Render() {
    for (int r = 0; r < S_ROWS; ++r)
        for (int c = 0; c < S_COLS; ++c) {
            SDL_Color col = ((r + c) & 1) ? COL_A : COL_B;
            Renderer::FillRect({ float(S_FRAME + c * S_CELL),float(S_FRAME + r * S_CELL) },
                S_CELL, S_CELL, col);
        }
    Renderer::FillRect({ 0,0 }, S_WIDTH, S_FRAME, { 128,128,128,255 });
    Renderer::FillRect({ 0,S_HEIGHT - S_FRAME }, S_WIDTH, S_FRAME, { 128,128,128,255 });
    Renderer::FillRect({ 0,0 }, S_FRAME, S_HEIGHT, { 128,128,128,255 });
    Renderer::FillRect({ S_WIDTH - S_FRAME,0 }, S_FRAME, S_HEIGHT, { 128,128,128,255 });

    Renderer::FillCircle(foodPos, S_FOOD_R, { 0,255,0,255 });

    SDL_Color dg{ 0,120,0,255 }, lg{ 0,180,0,255 };
    for (size_t i = 1; i < snake.size(); ++i) {
        Renderer::FillRect({ snake[i].x,snake[i].y }, S_CELL, S_CELL, ((i & 1) ? dg : lg));
    }

    Point2D hc{ snake[0].x + S_CELL / 2, snake[0].y + S_CELL / 2 };
    Renderer::FillCircle(hc, S_CELL / 2, { 0,200,0,255 });

    int eyeR = S_CELL / 8; float off = S_CELL * 0.25f;
    float ex1 = hc.x, ex2 = hc.x, ey1 = hc.y, ey2 = hc.y;
    switch (dir) {
    case 0: ey1 = ey2 = hc.y - off; ex1 = hc.x - eyeR * 2; ex2 = hc.x + eyeR * 2; break;
    case 1: ex1 = ex2 = hc.x + off; ey1 = hc.y - eyeR * 2; ey2 = hc.y + eyeR * 2; break;
    case 2: ey1 = ey2 = hc.y + off; ex1 = hc.x - eyeR * 2; ex2 = hc.x + eyeR * 2; break;
    case 3: ex1 = ex2 = hc.x - off; ey1 = hc.y - eyeR * 2; ey2 = hc.y + eyeR * 2; break;
    }
    Renderer::FillCircle({ ex1,ey1 }, eyeR, { 255,255,255,255 });
    Renderer::FillCircle({ ex2,ey2 }, eyeR, { 255,255,255,255 });
    Renderer::FillCircle({ ex1,ey1 }, eyeR / 2, { 0,0,0,255 });
    Renderer::FillCircle({ ex2,ey2 }, eyeR / 2, { 0,0,0,255 });
}
