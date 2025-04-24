#ifndef ENGINE_H
#define ENGINE_H

#include <SDL.h>
#include <vector>
#include "Point2D.h"
#include "Primitiv.h"
#include "Snake.h"        // <<–– gra w osobnych plikach

class Engine {
public:
    Engine();
    ~Engine();

    bool Init();
    void Run();
    void Shutdown();

private:
    // ----------------------------------------------------------------
    //  Zasoby SDL i stan g³ównej pêtli
    SDL_Window* window;
    SDL_Renderer* renderer;
    bool   isRunning;

    // ----------------------------------------------------------------
    //  Obiekty do rysowania przed startem Snake
    std::vector<Primitive>           primityw;
    std::vector<std::vector<Point2D>> unregular;

    // ----------------------------------------------------------------
    //  Drag & drop prymitywów
    bool    isDragging;
    int     selectedIndex;
    int     selectedUnregIndex;
    Point2D dragOffset;

    // ----------------------------------------------------------------
    //  Gra „Snake” w oddzielnej klasie
    bool       snakeRunning;   // flaga startu gry (Space)
    SnakeGame  snake;          // instancja gry

    // ----------------------------------------------------------------
    //  Funkcje pomocnicze silnika (kolizje dla drag-and-drop)
    bool IsInsideSquare(const Primitive& p, float mouseX, float mouseY);
    bool IsInsideCircle(const Primitive& p, float mouseX, float mouseY);
    bool IsInsideUnregular(const std::vector<Point2D>& pts, float mouseX, float mouseY);
};

#endif
