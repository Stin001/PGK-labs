#ifndef POINT2D_H
#define POINT2D_H

#include <cmath>        // sin, cos

struct Point2D {
    float x, y;

    constexpr Point2D(float x = 0.f, float y = 0.f) : x(x), y(y) {}

    // ───────────────────── pomocnicze operatory ─────────────────────
    constexpr Point2D operator+(const Point2D& o) const { return { x + o.x, y + o.y }; }
    constexpr Point2D operator-(const Point2D& o) const { return { x - o.x, y - o.y }; }

    // ────────────────────── przekształcenia 2-D ──────────────────────
    inline void translate(float dx, float dy) { x += dx; y += dy; }

    inline Point2D rotated(float deg, const Point2D& pivot) const
    {
        constexpr float PI = 3.14159265358979323846f;
        float  rad = deg * (PI / 180.f);
        float  s = std::sin(rad);
        float  c = std::cos(rad);
        float  xr = pivot.x + (x - pivot.x) * c - (y - pivot.y) * s;
        float  yr = pivot.y + (x - pivot.x) * s + (y - pivot.y) * c;
        return { xr, yr };
    }

    inline Point2D scaled(float kx, float ky, const Point2D& pivot) const
    {
        return { pivot.x + (x - pivot.x) * kx,
                 pivot.y + (y - pivot.y) * ky };
    }
};

#endif
