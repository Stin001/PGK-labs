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
    Point2D position;
    int width, height;
    SDL_Color color;

    // Jeœli chcemy przechowywaæ wierzcho³ki nieregularnego wielok¹ta w tym samym obiekcie
    // (niektórzy tak robi¹ – zamiast oddzielnego "unregular"), mo¿na to te¿ trzymaæ tutaj:
    std::vector<Point2D> points_unrl;

    Primitive()
        : type(PrimitiveType::KWADRAT), position(0, 0), width(50), height(50),
        color({ 255, 255, 255, 255 }) {
    }

    // konstruktor np. do kwadratu/kó³ka
    Primitive(PrimitiveType t, Point2D pos, int w, int h, SDL_Color col)
        : type(t), position(pos), width(w), height(h), color(col) {
    }

    // konstruktor np. do wielok¹ta
    Primitive(PrimitiveType t, const std::vector<Point2D>& pts, SDL_Color col)
        : type(t), points_unrl(pts), color(col) {
        // width/height czy position nie s¹ wtedy u¿ywane
    }
};

#endif
