#ifndef SNAKE_H
#define SNAKE_H

#include <vector>
#include "Point2D.h"
#include "Input.h"
#include "Renderer.h"

// ??????????????????????????? STA£E PLANSZY ???????????????????????????
constexpr int S_WIDTH = 800;
constexpr int S_HEIGHT = 600;
constexpr int S_FRAME = 20;             // gruboœæ szarej ramki
constexpr int S_CELL = 40;             // rozmiar jednej kratki
constexpr int S_COLS = (S_WIDTH - 2 * S_FRAME) / S_CELL;
constexpr int S_ROWS = (S_HEIGHT - 2 * S_FRAME) / S_CELL;
constexpr int S_FOOD_R = 16;             // promieñ kó³ka
// kolory pól szachownicy
constexpr SDL_Color COL_A{ 35, 35, 35, 255 };
constexpr SDL_Color COL_B{ 60, 60, 60, 255 };
// ??????????????????????????????????????????????????????????????????????

class SnakeGame {
public:
    SnakeGame();

    // Pêtla gry (wywo³uj kolejno w Engine):
    void HandleInput();   // przetwarza strza³ki, blokuje zawracanie
    void Update();        // ruch wê¿a, kolizje, jedzenie
    void Render();        // rysuje planszê, wê¿a i po¿ywienie

    bool IsAlive() const { return running; }
    void Restart();       // reset po ew. kolizji

private:
    struct Segment { float x, y; };
    std::vector<Segment> snake;
    Point2D foodPos;
    int  dir;                // 0? 1? 2? 3?
    int  moveCounter;        // spowalnia ruch
    bool running;            // true - gra trwa

    // pomocnicze
    void spawnFood();        // losuje nowe kó³ko
};

#endif // SNAKE_H
