#include "Snake.h"
#include <cstdlib>
#include <ctime>
#include <cmath>

// ──────────────────────────────────────────────────────────────────────
SnakeGame::SnakeGame()
    : dir(1), moveCounter(0), running(true)
{
    std::srand((unsigned)std::time(nullptr));
    Restart();
}
// ──────────────────────────────────────────────────────────────────────
void SnakeGame::Restart()
{
    snake.clear();
    // start – środek planszy, 3 segmenty
    const float cx = S_FRAME + (S_COLS / 2) * S_CELL;
    const float cy = S_FRAME + (S_ROWS / 2) * S_CELL;
    snake.push_back({ cx,          cy });
    snake.push_back({ cx - S_CELL, cy });
    snake.push_back({ cx - 2 * S_CELL, cy });
    dir = 1;
    running = true;
    spawnFood();
}
// ──────────────────────────────────────────────────────────────────────
void SnakeGame::spawnFood()
{
    foodPos.x = float(S_FRAME + (std::rand() % S_COLS) * S_CELL + S_CELL / 2);
    foodPos.y = float(S_FRAME + (std::rand() % S_ROWS) * S_CELL + S_CELL / 2);
}
// ──────────────────────────────────────────────────────────────────────
void SnakeGame::HandleInput()
{
    if (!running) return;

    int nd = dir;
    if (Input::IsKeyPressed(SDLK_UP))    nd = 0;
    if (Input::IsKeyPressed(SDLK_RIGHT)) nd = 1;
    if (Input::IsKeyPressed(SDLK_DOWN))  nd = 2;
    if (Input::IsKeyPressed(SDLK_LEFT))  nd = 3;

    bool opp = (dir == 0 && nd == 2) || (dir == 2 && nd == 0) ||
        (dir == 1 && nd == 3) || (dir == 3 && nd == 1);
    if (!opp) dir = nd;
}
// ──────────────────────────────────────────────────────────────────────
void SnakeGame::Update()
{
    if (!running) return;
    if (++moveCounter < 300) return;
    moveCounter = 0;

    // przesuwamy ogon
    for (int i = int(snake.size()) - 1; i > 0; --i) snake[i] = snake[i - 1];

    switch (dir) {
    case 0: snake[0].y -= S_CELL; break;
    case 1: snake[0].x += S_CELL; break;
    case 2: snake[0].y += S_CELL; break;
    case 3: snake[0].x -= S_CELL; break;
    }

    // kolizja z ramką
    if (snake[0].x < S_FRAME || snake[0].x >= S_FRAME + S_COLS * S_CELL ||
        snake[0].y < S_FRAME || snake[0].y >= S_FRAME + S_ROWS * S_CELL) {
        running = false; return;
    }

    // kolizja z ciałem
    for (size_t i = 1; i < snake.size(); ++i)
        if (snake[0].x == snake[i].x && snake[0].y == snake[i].y) {
            running = false; return;
        }

    // zjedzenie kółka
    float dx = (snake[0].x + S_CELL / 2) - foodPos.x;
    float dy = (snake[0].y + S_CELL / 2) - foodPos.y;
    float sumR = S_CELL / 2 + S_FOOD_R;
    if (dx * dx + dy * dy <= sumR * sumR) {
        snake.push_back(snake.back());   // wydłuż
        spawnFood();
    }
}
// ──────────────────────────────────────────────────────────────────────
void SnakeGame::Render()
{
    // szachownica
    for (int r = 0; r < S_ROWS; ++r)
        for (int c = 0; c < S_COLS; ++c) {
            SDL_Color col = ((r + c) & 1) ? COL_A : COL_B;
            Renderer::FillRect({ float(S_FRAME + c * S_CELL),
                                 float(S_FRAME + r * S_CELL) },
                S_CELL, S_CELL, col);
        }
    // ramka
    Renderer::FillRect({ 0,0 }, S_WIDTH, S_FRAME, { 128,128,128,255 });
    Renderer::FillRect({ 0,S_HEIGHT - S_FRAME }, S_WIDTH, S_FRAME, { 128,128,128,255 });
    Renderer::FillRect({ 0,0 }, S_FRAME, S_HEIGHT, { 128,128,128,255 });
    Renderer::FillRect({ S_WIDTH - S_FRAME,0 }, S_FRAME, S_HEIGHT, { 128,128,128,255 });

    // jedzenie
    Renderer::FillCircle(foodPos, S_FOOD_R, { 0,255,0,255 });

    // wąż
    for (const auto& s : snake)
        Renderer::FillRect({ s.x,s.y }, S_CELL, S_CELL, { 255,0,0,255 });
}
// ──────────────────────────────────────────────────────────────────────
