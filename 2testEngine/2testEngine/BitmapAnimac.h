#pragma once

#include <SDL.h>
#include <string>
#include "Point2D.h"

// Klasa reprezentująca bitmapę (teksturę)
class Bitmap {
public:
    SDL_Texture* texture{ nullptr };
    int width{ 0 }, height{ 0 };

    ~Bitmap() {
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
        if (!texture) SDL_Log("CreateTexture error: %s", SDL_GetError());
        return texture != nullptr;
    }

    void destroy() {
        if (texture) {
            SDL_DestroyTexture(texture);
            texture = nullptr;
        }
        width = height = 0;
    }

    void render(SDL_Renderer* r, int x, int y, int srcX = 0, int srcY = 0, int srcW = -1, int srcH = -1) const {
        if (!texture) return;

        SDL_Rect srcRect = { srcX, srcY, srcW == -1 ? width : srcW, srcH == -1 ? height : srcH };
        SDL_Rect dstRect = { x, y, width, height };
        SDL_RenderCopy(r, texture, &srcRect, &dstRect);
    }
};

// Klasa bazowa dla obiektów animowanych
class AnimatedObject {
public:
    virtual void animate() = 0; // Wirtualna funkcja do animacji
};

// Klasa reprezentująca obiekt bitmapy z animacją
class SpriteObject : public Bitmap, public AnimatedObject {
public:
    Point2D position;          // Pozycja bitmapy na ekranie
    bool isAnimating;          // Czy bitmapa jest animowana
    Point2D velocity;          // Prędkość animacji (współrzędne delta)

    int currentFrame = 0;      // Indeks obecnej klatki
    int totalFrames = 0;       // Całkowita liczba klatek w animacji
    int frameWidth = 0;        // Szerokość pojedynczej klatki
    int frameHeight = 0;       // Wysokość pojedynczej klatki
    int frameDelay = 10;       // Opóźnienie między klatkami (klatki na sekundę)
    int frameCounter = 0;      // Licznik klatek

    int dir = 1;               // Kierunek węża (0: góra, 1: prawo, 2: dół, 3: lewo)

    SpriteObject()
        : isAnimating(false), velocity(0, 0) {
    }

    bool load(SDL_Renderer* renderer, const std::string& path, const Point2D& startPos, int totalFrames, int frameWidth, int frameHeight) {
        position = startPos;
        this->totalFrames = totalFrames;
        this->frameWidth = frameWidth;
        this->frameHeight = frameHeight;
        return Bitmap::load(renderer, path);
    }

    void startAnimation(const Point2D& vel) {
        isAnimating = true;
        velocity = vel;
    }

    void stopAnimation() {
        isAnimating = false;
        velocity = Point2D(0, 0);
    }

    // Aktualizacja pozycji bitmapy i przełączanie klatek
    void update() override {
        if (isAnimating) {
            position.x += velocity.x;
            position.y += velocity.y;

            // Przełączanie klatki animacji
            if (++frameCounter >= frameDelay) {
                frameCounter = 0;
                currentFrame = (currentFrame + 1) % totalFrames;
            }
        }
    }

    // Renderowanie aktualnej klatki
    void render(SDL_Renderer* r) override {
        int srcX = currentFrame * frameWidth; // Oblicz X dla bieżącej klatki
        int srcY = 0; // W tym przypadku wszystkie klatki w jednej linii
        Bitmap::render(r, int(position.x), int(position.y), srcX, srcY, frameWidth, frameHeight);
    }

    void animate() override {
        // W zależności od kierunku ruchu zmieniamy klatki animacji
        switch (dir) {
        case 0: // Ruch w górę
            currentFrame = 0;
            break;
        case 1: // Ruch w prawo
            currentFrame = 1;
            break;
        case 2: // Ruch w dół
            currentFrame = 2;
            break;
        case 3: // Ruch w lewo
            currentFrame = 3;
            break;
        }
        update();  // Aktualizuj pozycję
    }
};
