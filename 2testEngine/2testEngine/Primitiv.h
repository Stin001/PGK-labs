#ifndef PRIMITIVE_H
#define PRIMITIVE_H

#include <vector>
#include <SDL.h>
#include "Point2D.h"

enum class PrimitiveType {
    KWADRAT,
    LINE,
    CIRCLE,
    UNREGULAR
};

struct Primitive {
    PrimitiveType type;
    Point2D       position;
    int           width, height;
    SDL_Color     color;
    float         angle{ 0.f };
    std::vector<Point2D> points_unrl;

    Primitive()
        : type(PrimitiveType::KWADRAT),
        position(0, 0),
        width(50), height(50),
        color({ 255,255,255,255 })
    {
    }

    Primitive(PrimitiveType t, Point2D pos, int w, int h, SDL_Color col)
        : type(t), position(pos), width(w), height(h), color(col)
    {
    }

    Primitive(PrimitiveType t, const std::vector<Point2D>& pts, SDL_Color col)
        : type(t), points_unrl(pts), color(col)
    {
    }
};

#endif // PRIMITIVE_H
