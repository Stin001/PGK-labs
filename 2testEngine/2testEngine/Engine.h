#ifndef ENGINE_H
#define ENGINE_H

#include <SDL.h>
#include <vector>
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

    // Bitmapy
    struct Sprite {
        Bitmap bmp;
        Point2D pos;
    };

    // Kontener przechowujący bitmapy (BitmapObject)
    std::vector<BitmapObject> bitmapObjects;

    // Przechowywanie innych obiektów
    std::vector<Primitive>            primityw;
    std::vector<std::vector<Point2D>> unregular;

    // Snake
    bool       snakeRunning{ false };
    SnakeGame  snake;

private:
    SDL_Window* window{};            // Wskaźnik do okna SDL
    SDL_Renderer* renderer{};        // Wskaźnik do renderer'a SDL
    bool          isRunning{ false }; // Stan silnika (czy działa)

    // Tryb wstawiania obiektów
    enum class CreateMode { NONE, KWADRAT, UNREGULAR, CIRCLE, BITMAP };
    CreateMode createMode{ CreateMode::NONE };

    // Do przeciągania obiektów
    bool    isDragging{ false };
    int     selectedIndex{ -1 };
    int     selectedUnregIndex{ -1 };
    Point2D dragOffset{};

    // Metody pomocnicze do wykrywania kolizji
    bool IsInsideSquare(const Primitive& p, float mx, float my);
    bool IsInsideCircle(const Primitive& p, float mx, float my);
    bool IsInsideUnregular(const std::vector<Point2D>& pts, float mx, float my);

    // Przechwytywanie wejścia (np. z klawiatury, myszy)
    void HandleInput(const SDL_Event& ev);
};

#endif // ENGINE_H
