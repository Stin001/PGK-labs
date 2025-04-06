#ifndef ENGINE_H
#define ENGINE_H

#include <SDL.h>
#include <vector>
#include "Point2D.h"
#include "Primitiv.h"

struct SnakeSegment { // bez zmian
    float x, y;
};

class Engine {
public:
    Engine();
    ~Engine();

    bool Init();
    void Run();
    void Shutdown();

private:
    SDL_Window* window;
    SDL_Renderer* renderer;
    bool isRunning;

    // Przed wciœniêciem spacji - stare obiekty
    std::vector<Primitive> primityw;
    std::vector<std::vector<Point2D>> unregular;

    // Drag & drop
    bool isDragging;
    int  selectedIndex;
    int  selectedUnregIndex;
    Point2D dragOffset;

    // ----------------------------
    // W¹¿
    bool snakeRunning;
    int  snakeDir;
    int  snakeMoveCounter;
    std::vector<SnakeSegment> snake;
    Point2D foodPos;

private:
    bool IsInsideSquare(const Primitive& p, float mouseX, float mouseY);
    bool IsInsideCircle(const Primitive& p, float mouseX, float mouseY);
    bool IsInsideUnregular(const std::vector<Point2D>& pts, float mouseX, float mouseY);

    void InitSnake();
    void UpdateSnake();
    void RenderSnake();
};

#endif
