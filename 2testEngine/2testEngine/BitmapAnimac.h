#pragma once

#include <SDL.h>
#include <string>
#include "Point2D.h"

// -----------------------------------------------------------------------------
// Bitmap — baza dla wszystkich obiektów, które potrafią się renderować
// -----------------------------------------------------------------------------
class Bitmap {
public:
    SDL_Texture* texture{ nullptr };
    int width{ 0 }, height{ 0 };

    virtual ~Bitmap() {
        destroy();
    }

    bool load(SDL_Renderer* renderer, const std::string& path) {
        destroy();
        SDL_Surface* surf = SDL_LoadBMP(path.c_str());
        if (!surf) {
            SDL_Log("SDL_LoadBMP error: %s", SDL_GetError());
            return false;
        }
        texture = SDL_CreateTextureFromSurface(renderer, surf);
        width = surf->w;
        height = surf->h;
        SDL_FreeSurface(surf);
        if (!texture) {
            SDL_Log("CreateTexture error: %s", SDL_GetError());
            return false;
        }
        return true;
    }

    void destroy() {
        if (texture) {
            SDL_DestroyTexture(texture);
            texture = nullptr;
        }
        width = height = 0;
    }

    // główna, wirtualna metoda renderująca
    virtual void render(SDL_Renderer* r) const {
        // do nadpisania w klasie pochodnej
    }

    // overload do fragmentu bitmapy
    void render(SDL_Renderer* r,
        int x, int y,
        int srcX = 0, int srcY = 0,
        int srcW = -1, int srcH = -1) const
    {
        if (!texture) return;
        SDL_Rect src{ srcX, srcY,
                      srcW == -1 ? width : srcW,
                      srcH == -1 ? height : srcH };
        SDL_Rect dst{ x, y, width, height };
        SDL_RenderCopy(r, texture, &src, &dst);
    }
};

// -----------------------------------------------------------------------------
// AnimatedObject — dla rzeczy, które mają metodę animate()
// -----------------------------------------------------------------------------
class AnimatedObject {
public:
    virtual ~AnimatedObject() = default;
    virtual void animate() = 0;
};

// -----------------------------------------------------------------------------
// SpriteObject — bitmapa + prosta animacja klatkowa
// -----------------------------------------------------------------------------
class SpriteObject : public Bitmap, public AnimatedObject {
public:
    Point2D position{ 0,0 }, velocity{ 0,0 };
    bool    isAnimating{ false };

    int currentFrame{ 0 }, totalFrames{ 1 };
    int frameWidth{ 0 }, frameHeight{ 0 };
    int frameDelay{ 8 }, frameCounter{ 0 };

    SpriteObject() = default;

    bool load(SDL_Renderer* renderer,
        const std::string& path,
        const Point2D& startPos,
        int tf, int fw, int fh)
    {
        position = startPos;
        totalFrames = tf;
        frameWidth = fw;
        frameHeight = fh;
        return Bitmap::load(renderer, path);
    }

    void startAnimation(const Point2D& vel) {
        isAnimating = true;
        velocity = vel;
    }
    void stopAnimation() {
        isAnimating = false;
        velocity = { 0,0 };
    }

    void animate() override {
        if (!isAnimating) return;
        position = position + velocity;
        if (++frameCounter >= frameDelay) {
            frameCounter = 0;
            currentFrame = (currentFrame + 1) % totalFrames;
        }
    }

    void render(SDL_Renderer* r) const override {
        if (!texture) return;
        int sx = currentFrame * frameWidth;
        Bitmap::render(r,
            int(position.x),
            int(position.y),
            sx, 0,
            frameWidth, frameHeight);
    }
};
