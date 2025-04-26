#pragma once
#include "BitmapAnimac.h"
#include <vector>
#include <SDL.h>

class SpriteAnimation {
    std::vector<Bitmap> frames;
    int currentFrame{ 0 };
    Uint32 frameDelay;      // czas miêdzy klatkami w ms
    Uint32 lastTick{ 0 };     // SDL_GetTicks() z ostatniej zmiany

public:
    SpriteAnimation(int delayMs) : frameDelay(delayMs), lastTick(SDL_GetTicks()) {}

    // dodaj klatkê z pliku BMP
    void addFrame(const std::string& path, SDL_Renderer* rnd) {
        Bitmap bmp;
        if (bmp.load(rnd, path))
            frames.push_back(std::move(bmp));
    }

    // wywo³uj co klatkê w Update()
    void update() {
        if (frames.empty()) return;
        Uint32 now = SDL_GetTicks();
        if (now - lastTick >= (Uint32)frameDelay) {
            currentFrame = (currentFrame + 1) % frames.size();
            lastTick = now;
        }
    }

    // rysuj aktualn¹ klatkê
    void render(SDL_Renderer* rnd, int x, int y) const {
        if (!frames.empty())
            frames[currentFrame].render(rnd, x, y);
    }
};
