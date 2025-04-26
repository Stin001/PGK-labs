#ifndef SNAKE_H
#define SNAKE_H

#include <vector>
#include "Point2D.h"
#include "Input.h"
#include "Renderer.h"
#include <SDL.h>

constexpr int S_WIDTH = 800;
constexpr int S_HEIGHT = 600;
constexpr int S_FRAME = 20;
constexpr int S_CELL = 40;
constexpr int S_COLS = (S_WIDTH - 2 * S_FRAME) / S_CELL;
constexpr int S_ROWS = (S_HEIGHT - 2 * S_FRAME) / S_CELL;
constexpr int S_FOOD_R = 16;

inline const SDL_Color COL_A{ 35,120,35,255 };
inline const SDL_Color COL_B{ 25,95,25,255 };

class SnakeGame {
public:
    SnakeGame();

    void HandleInput();
    void Update();
    void Render();

    bool IsAlive() const { return running; }
    void Restart();

private:
    struct Segment { float x, y; };
    std::vector<Segment> snake;
    Point2D foodPos;
    int dir, moveCounter;
    bool running;

    void spawnFood();
};

#endif // SNAKE_H
