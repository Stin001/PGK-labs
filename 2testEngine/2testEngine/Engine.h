#ifndef ENGINE_H
#define ENGINE_H

#include <SDL.h>
#include <vector>
#include "Point2D.h"
#include "Primitiv.h"
#include "Snake.h"
#include "BitmapAnimac.h"            // ← dodane

class Engine {
public:
    Engine();
    ~Engine();

    bool Init();
    void Run();
    void Shutdown();

private:
    SDL_Window* window{};
    SDL_Renderer* renderer{};
    bool          isRunning{ false };

    // prymitywy
    std::vector<Primitive>            primityw;
    std::vector<std::vector<Point2D>> unregular;

    // bitmapy
    struct Sprite { Bitmap bmp; Point2D pos; };
    std::vector<Sprite> sprites;      // ← nowy kontener

    // przeciąganie
    bool    isDragging{ false };
    int     selectedIndex{ -1 };
    int     selectedUnregIndex{ -1 };
    Point2D dragOffset{};

    // tryb wstawiania 1/2/3/4
    enum class CreateMode { NONE, KWADRAT, UNREGULAR, CIRCLE, BITMAP };
    CreateMode createMode{ CreateMode::NONE };

    // Snake
    bool       snakeRunning{ false };
    SnakeGame  snake;

    // kolizje
    bool IsInsideSquare(const Primitive& p, float mx, float my);
    bool IsInsideCircle(const Primitive& p, float mx, float my);
    bool IsInsideUnregular(const std::vector<Point2D>& pts, float mx, float my);
};

#endif
