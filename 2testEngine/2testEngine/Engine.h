#ifndef ENGINE_H
#define ENGINE_H

#include <SDL.h>
#include <vector>
#include "Point2D.h"
#include "Primitiv.h"

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

    // Wektory obiektów:
    std::vector<Primitive> primityw;              // kwadraty, kó³ka itp.
    std::vector<std::vector<Point2D>> unregular;  // nieregularne wielok¹ty

    // Obs³uga przesuwania (drag & drop):
    bool isDragging;
    int  selectedIndex;       // który element w "primityw" jest wybrany
    int  selectedUnregIndex;  // który wielok¹t w "unregular" jest wybrany
    Point2D dragOffset;       // zapamiêtujemy poprzedni¹ pozycjê myszy w trakcie przeci¹gania

private:
    // Funkcje pomocnicze do wykrycia klikniêcia w dany kszta³t
    bool IsInsideSquare(const Primitive& p, float mouseX, float mouseY);
    bool IsInsideCircle(const Primitive& p, float mouseX, float mouseY);
    bool IsInsideUnregular(const std::vector<Point2D>& pts, float mouseX, float mouseY);
};

#endif
