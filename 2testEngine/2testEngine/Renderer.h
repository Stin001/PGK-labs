
#ifndef RENDERER_H
#define RENDERER_H
#include <algorithm>

#include <SDL.h>
#include "Point2D.h"
#include "Primitiv.h"
class Renderer {
public:
    static void Init(SDL_Renderer* renderer);
    static void DrawPoint(const Point2D& point, SDL_Color color);
    static void DrawLine(const Point2D& start, const Point2D& end, SDL_Color color);
    static void DrawRect(const Point2D& position, int width, int height, SDL_Color color);
    static void FillRect(const Point2D& position, int width, int height, SDL_Color color);
    static void DrawUnregular(const std::vector<Point2D>& points, SDL_Color color);
    static void UnregularFill(const std::vector<Point2D>& points, const SDL_Color& color);

private:
    static SDL_Renderer* sdlRenderer;
}; 

#endif
