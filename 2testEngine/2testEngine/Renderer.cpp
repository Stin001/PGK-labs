#include "Renderer.h"
#include <algorithm>
#include <cmath>
#include <iostream>

SDL_Renderer* Renderer::sdlRenderer = nullptr;

void Renderer::Init(SDL_Renderer* renderer) {
    sdlRenderer = renderer;
}

// ---------------------------------------------------------------------
// PODSTAWOWE PRYMITYWY

void Renderer::DrawPoint(const Point2D& point, SDL_Color color) {
    SDL_SetRenderDrawColor(sdlRenderer, color.r, color.g, color.b, color.a);
    SDL_RenderDrawPoint(sdlRenderer, (int)point.x, (int)point.y);
}

void Renderer::DrawLine(const Point2D& start, const Point2D& end, SDL_Color color) {
    SDL_SetRenderDrawColor(sdlRenderer, color.r, color.g, color.b, color.a);
    SDL_RenderDrawLine(sdlRenderer,
        (int)start.x, (int)start.y,
        (int)end.x, (int)end.y);
}

void Renderer::DrawRect(const Point2D& position, int width, int height, SDL_Color color) {
    SDL_SetRenderDrawColor(sdlRenderer, color.r, color.g, color.b, color.a);
    SDL_Rect rect = { (int)position.x, (int)position.y, width, height };
    SDL_RenderDrawRect(sdlRenderer, &rect);
}

void Renderer::FillRect(const Point2D& position, int width, int height, SDL_Color color) {
    SDL_SetRenderDrawColor(sdlRenderer, color.r, color.g, color.b, color.a);
    SDL_Rect rect = { (int)position.x, (int)position.y, width, height };
    SDL_RenderFillRect(sdlRenderer, &rect);
}

// ---------------------------------------------------------------------
// WIELOK¥T NIEREGULARNY

void Renderer::DrawUnregular(const std::vector<Point2D>& points, SDL_Color color) {
    if (points.size() < 2) return;
    SDL_SetRenderDrawColor(sdlRenderer, color.r, color.g, color.b, color.a);

    // rysowanie kolejnych krawêdzi
    for (size_t i = 0; i < points.size() - 1; ++i) {
        SDL_RenderDrawLine(sdlRenderer,
            (int)points[i].x, (int)points[i].y,
            (int)points[i + 1].x, (int)points[i + 1].y
        );
    }
    // zamkniêcie (ostatni -> pierwszy)
    SDL_RenderDrawLine(sdlRenderer,
        (int)points[points.size() - 1].x, (int)points[points.size() - 1].y,
        (int)points[0].x, (int)points[0].y
    );
}

void Renderer::UnregularFill(const std::vector<Point2D>& points, const SDL_Color& color) {
    if (points.size() < 3) return; // minimum 3 wierzcho³ki

    SDL_SetRenderDrawColor(sdlRenderer, color.r, color.g, color.b, color.a);

    // znajdŸ minY, maxY
    int minY = (int)points[0].y;
    int maxY = (int)points[0].y;
    for (const auto& p : points) {
        if (p.y < minY) minY = (int)p.y;
        if (p.y > maxY) maxY = (int)p.y;
    }

    // scan-line
    for (int y = minY; y <= maxY; y++) {
        std::vector<int> interX;

        for (size_t i = 0; i < points.size(); i++) {
            size_t j = (i + 1) % points.size();
            int x1 = (int)points[i].x, y1 = (int)points[i].y;
            int x2 = (int)points[j].x, y2 = (int)points[j].y;

            // Sprawdzamy, czy pozioma linia y przecina krawêdŸ [i->j]
            if ((y1 <= y && y2 > y) || (y2 <= y && y1 > y)) {
                float x = (float)x1 + (float)(y - y1) * (float)(x2 - x1) / (float)(y2 - y1);
                interX.push_back((int)x);
            }
        }

        std::sort(interX.begin(), interX.end());

        // rysujemy odcinki miêdzy parami x-ów
        for (size_t i = 0; i + 1 < interX.size(); i += 2) {
            SDL_RenderDrawLine(sdlRenderer, interX[i], y, interX[i + 1], y);
        }
    }
}

// ---------------------------------------------------------------------
// KÓ£KO

void Renderer::DrawCircle(const Point2D& center, int radius, SDL_Color color) {
    SDL_SetRenderDrawColor(sdlRenderer, color.r, color.g, color.b, color.a);

    const int segments = 360;
    float step = 2.0f * 3.14159265359f / segments;

    for (int i = 0; i < segments; i++) {
        float theta1 = i * step;
        float theta2 = (i + 1) * step;

        int x1 = (int)(center.x + radius * cos(theta1));
        int y1 = (int)(center.y + radius * sin(theta1));
        int x2 = (int)(center.x + radius * cos(theta2));
        int y2 = (int)(center.y + radius * sin(theta2));

        SDL_RenderDrawLine(sdlRenderer, x1, y1, x2, y2);
    }
}

void Renderer::FillCircle(const Point2D& center, int radius, SDL_Color color) {
    SDL_SetRenderDrawColor(sdlRenderer, color.r, color.g, color.b, color.a);

    for (int dy = -radius; dy <= radius; dy++) {
        int dxMax = (int)std::sqrt((float)(radius * radius - dy * dy));
        int cy = (int)center.y + dy;
        int cx1 = (int)center.x - dxMax;
        int cx2 = (int)center.x + dxMax;
        SDL_RenderDrawLine(sdlRenderer, cx1, cy, cx2, cy);
    }
}
