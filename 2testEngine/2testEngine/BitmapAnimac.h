#pragma once
#include <SDL.h>
#include <string>

/**
 *  Prosty loader BMP → SDL_Texture + renderowanie.
 */
class Bitmap {
    SDL_Texture* texture{ nullptr };
    int width{ 0 }, height{ 0 };
public:
    ~Bitmap() { destroy(); }

    // ładuje czystego BMP-a, zwraca true jeśli ok
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

    // renderuje w (x,y) w oryginalnym rozmiarze
    void render(SDL_Renderer* r, int x, int y) const {
        if (!texture) return;
        SDL_Rect dst{ x, y, width, height };
        SDL_RenderCopy(r, texture, nullptr, &dst);
    }
};
