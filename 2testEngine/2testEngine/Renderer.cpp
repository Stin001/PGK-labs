#include "Renderer.h"
#include <algorithm>
#include <cmath>

SDL_Renderer* Renderer::sdlRenderer = nullptr;

void Renderer::Init(SDL_Renderer* renderer) {
    sdlRenderer = renderer;
}

// podstawowe prymitywy:

void Renderer::DrawPoint(const Point2D& p, SDL_Color c) {
    SDL_SetRenderDrawColor(sdlRenderer, c.r, c.g, c.b, c.a);
    SDL_RenderDrawPoint(sdlRenderer, int(p.x), int(p.y));
}

void Renderer::DrawLine(const Point2D& a, const Point2D& b, SDL_Color c) {
    SDL_SetRenderDrawColor(sdlRenderer, c.r, c.g, c.b, c.a);
    SDL_RenderDrawLine(sdlRenderer, int(a.x), int(a.y), int(b.x), int(b.y));
}

void Renderer::DrawRect(const Point2D& pos, int w, int h, SDL_Color c) {
    SDL_SetRenderDrawColor(sdlRenderer, c.r, c.g, c.b, c.a);
    SDL_Rect r{ int(pos.x), int(pos.y), w, h };
    SDL_RenderDrawRect(sdlRenderer, &r);
}

void Renderer::FillRect(const Point2D& pos, int w, int h, SDL_Color c) {
    SDL_SetRenderDrawColor(sdlRenderer, c.r, c.g, c.b, c.a);
    SDL_Rect r{ int(pos.x), int(pos.y), w, h };
    SDL_RenderFillRect(sdlRenderer, &r);
}

// nieregularne wielok¹ty:

void Renderer::DrawUnregular(const std::vector<Point2D>& pts, SDL_Color c) {
    if (pts.size() < 2) return;
    SDL_SetRenderDrawColor(sdlRenderer, c.r, c.g, c.b, c.a);
    for (size_t i = 0; i + 1 < pts.size(); ++i)
        SDL_RenderDrawLine(sdlRenderer,
            int(pts[i].x), int(pts[i].y),
            int(pts[i + 1].x), int(pts[i + 1].y));
    SDL_RenderDrawLine(sdlRenderer,
        int(pts.back().x), int(pts.back().y),
        int(pts.front().x), int(pts.front().y));
}

void Renderer::UnregularFill(const std::vector<Point2D>& pts, const SDL_Color& c) {
    if (pts.size() < 3) return;
    SDL_SetRenderDrawColor(sdlRenderer, c.r, c.g, c.b, c.a);
    int minY = int(pts[0].y), maxY = int(pts[0].y);
    for (auto& p : pts) {
        minY = std::min(minY, int(p.y));
        maxY = std::max(maxY, int(p.y));
    }
    for (int y = minY; y <= maxY; ++y) {
        std::vector<int> xs;
        for (size_t i = 0; i < pts.size(); ++i) {
            size_t j = (i + 1) % pts.size();
            int x1 = int(pts[i].x), y1 = int(pts[i].y);
            int x2 = int(pts[j].x), y2 = int(pts[j].y);
            if ((y1 <= y && y2 > y) || (y2 <= y && y1 > y)) {
                float x = x1 + float(y - y1) * (x2 - x1) / float(y2 - y1);
                xs.push_back(int(x));
            }
        }
        std::sort(xs.begin(), xs.end());
        for (size_t i = 0; i + 1 < xs.size(); i += 2)
            SDL_RenderDrawLine(sdlRenderer, xs[i], y, xs[i + 1], y);
    }
}

// kó³ko:

void Renderer::DrawCircle(const Point2D& cen, int r, SDL_Color c) {
    SDL_SetRenderDrawColor(sdlRenderer, c.r, c.g, c.b, c.a);
    const int segs = 360;
    float step = 2.f * 3.14159265359f / segs;
    for (int i = 0; i < segs; ++i) {
        float t1 = i * step, t2 = (i + 1) * step;
        int x1 = int(cen.x + r * std::cos(t1));
        int y1 = int(cen.y + r * std::sin(t1));
        int x2 = int(cen.x + r * std::cos(t2));
        int y2 = int(cen.y + r * std::sin(t2));
        SDL_RenderDrawLine(sdlRenderer, x1, y1, x2, y2);
    }
}

void Renderer::FillCircle(const Point2D& cen, int r, SDL_Color c) {
    SDL_SetRenderDrawColor(sdlRenderer, c.r, c.g, c.b, c.a);
    for (int dy = -r; dy <= r; ++dy) {
        int dx = int(std::sqrt(float(r * r - dy * dy)));
        SDL_RenderDrawLine(sdlRenderer,
            int(cen.x) - dx, int(cen.y) + dy,
            int(cen.x) + dx, int(cen.y) + dy);
    }
}

void Renderer::FillRectRot(const Point2D& pos, int w, int h, float a, SDL_Color c) {
    SDL_SetRenderDrawColor(sdlRenderer, c.r, c.g, c.b, c.a);
    Point2D ctr{ pos.x + w * 0.5f, pos.y + h * 0.5f };
    Point2D pts[4] = {
        {pos.x,     pos.y},
        {pos.x + w, pos.y},
        {pos.x + w, pos.y + h},
        {pos.x,     pos.y + h}
    };
    for (auto& p : pts) p = p.rotated(a, ctr);
    UnregularFill({ pts[0],pts[1],pts[2],pts[3] }, c);
    for (int i = 0; i < 4; ++i) {
        auto& A = pts[i], & B = pts[(i + 1) & 3];
        SDL_RenderDrawLine(sdlRenderer,
            int(A.x), int(A.y),
            int(B.x), int(B.y));
    }
}
