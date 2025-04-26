#ifndef ENGINE_H
#define ENGINE_H

#include <SDL.h>
#include <vector>
#include <string>
#include "Point2D.h"
#include "Primitiv.h"
#include "Snake.h"
#include "BitmapAnimac.h"

class Engine {
public:
    Engine();
    ~Engine();

    bool Init();
    void Run();
    void Shutdown();

    void addBitmapObject(const std::string& path, const Point2D& position);
    void updateBitmapObjects();
    void renderBitmapObjects();

    // kontener spriteów
    std::vector<SpriteObject> bitmapObjects;

    // prymitywy
    std::vector<Primitive>            primityw;
    std::vector<std::vector<Point2D>> unregular;

    // Snake
    bool      snakeRunning{ false };
    SnakeGame snake;

private:
    SDL_Window* window{ nullptr };
    SDL_Renderer* renderer{ nullptr };
    bool          isRunning{ false };

    enum class CreateMode { NONE, KWADRAT, UNREGULAR, CIRCLE, BITMAP };
    CreateMode createMode{ CreateMode::NONE };

    bool    isDragging{ false };
    int     selectedIndex{ -1 };
    int     selectedUnregIndex{ -1 };
    Point2D dragOffset{};

    bool IsInsideSquare(const Primitive& p, float mx, float my);
    bool IsInsideCircle(const Primitive& p, float mx, float my);
    bool IsInsideUnregular(const std::vector<Point2D>& pts, float mx, float my);

    void HandleInput(const SDL_Event& ev);
};

#endif // ENGINE_H
