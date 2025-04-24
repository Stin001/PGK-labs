#ifndef ENGINE_H
#define ENGINE_H

#include <SDL.h>
#include <vector>
#include "Point2D.h"
#include "Primitiv.h"
#include "Snake.h"

// ??????????????????????????????????????????????????????????????
class Engine {
public:
    Engine();
    ~Engine();

    bool Init();
    void Run();
    void Shutdown();

private:
    // SDL
    SDL_Window* window;
    SDL_Renderer* renderer;
    bool          isRunning{ false };

    // Rysowane obiekty
    std::vector<Primitive>            primityw;    // kwadraty + kó³ka
    std::vector<std::vector<Point2D>> unregular;   // nieregularne wielok¹ty

    // Drag & drop
    bool    isDragging{ false };
    int     selectedIndex{ -1 };
    int     selectedUnregIndex{ -1 };
    Point2D dragOffset{};

    // „Tryb wstawiania” – ustawiany klawiszem 1 / 2 / 3
    enum class CreateMode { NONE, KWADRAT, UNREGULAR, CIRCLE };
    CreateMode createMode{ CreateMode::NONE };

    // Gra Snake
    bool       snakeRunning{ false };
    SnakeGame  snake;

    // Pomocnicze
    bool IsInsideSquare(const Primitive& p, float mx, float my);
    bool IsInsideCircle(const Primitive& p, float mx, float my);
    bool IsInsideUnregular(const std::vector<Point2D>& pts, float mx, float my);
};

#endif
