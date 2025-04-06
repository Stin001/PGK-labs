#include "Engine.h"
#include "Input.h"
#include "Renderer.h"
#include <iostream>
#include "Primitiv.h"
#include <vector>

Engine::Engine() : window(nullptr), renderer(nullptr), isRunning(false) {}

Engine::~Engine() {
    Shutdown();
}

bool Engine::Init() {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) {
        std::cerr << "B³¹d inicjalizacji SDL: " << SDL_GetError() << std::endl;
        return false;
    }

    window = SDL_CreateWindow("Silnik 2D", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        800, 600, SDL_WINDOW_SHOWN);
    if (!window) {
        std::cerr << "B³¹d tworzenia okna: " << SDL_GetError() << std::endl;
        return false;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::cerr << "B³¹d tworzenia renderera: " << SDL_GetError() << std::endl;
        return false;
    }

    Renderer::Init(renderer);
    isRunning = true;
    return true;
}

void Engine::Run() {
    SDL_Event event;
    while (isRunning) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                isRunning = false;
            }
            Input::HandleEvent(event);
        }

        if (Input::IsKeyPressed(SDLK_ESCAPE)) {
            isRunning = false;
        }

        SDL_RenderClear(renderer);

        // ---------------------------------
        // Tworzenie kwadratu po wciœniêciu '1' i klikniêciu mysz¹
        if (Input::IsKeyPressed(SDLK_1) && Input::IsMouseButtonPressed(SDL_BUTTON_LEFT)) {
            Point2D mousePos = Input::getMausPos();
            primityw.push_back(
                Primitive(PrimitiveType::KWADRAT, mousePos, 50, 50, { 255, 0, 0, 255 })
            );
        }

        // Rysowanie wszystkich kwadratów
        for (auto& p : primityw) {
            if (p.type == PrimitiveType::KWADRAT) {
                Renderer::FillRect(p.position, p.width, p.height, p.color);
            }
        }

        // ---------------------------------
        // Tworzenie nieregularnego wielok¹ta po wciœniêciu '2' i klikniêciu mysz¹
        if (Input::IsKeyPressed(SDLK_2) && Input::IsMouseButtonPressed(SDL_BUTTON_LEFT)) {
            Point2D mousePos = Input::getMausPos();

            // Przyk³adowy nieregularny wielok¹t o wierzcho³kach w okolicy klikniêcia
            std::vector<Point2D> points = {
                {mousePos.x,     mousePos.y},
                {mousePos.x + 40, mousePos.y},
                {mousePos.x + 60, mousePos.y + 30},
                {mousePos.x + 30, mousePos.y + 60},
                {mousePos.x,     mousePos.y + 30}
            };
            unregular.push_back(points);
        }

        // Rysowanie wszystkich nieregularnych wielok¹tów
        for (const auto& points : unregular) {
            Renderer::UnregularFill(points, { 255, 0, 0, 255 });
            Renderer::DrawUnregular(points, { 255, 0, 0, 255 });
        }

        // ---------------------------------
        // Tworzenie kó³ka po wciœniêciu '3' i klikniêciu mysz¹
        if (Input::IsKeyPressed(SDLK_3) && Input::IsMouseButtonPressed(SDL_BUTTON_LEFT)) {
            Point2D mousePos = Input::getMausPos();
            // width wykorzystamy jako "œrednicê" lub "œrednicê i height" - zale¿nie od implementacji
            primityw.push_back(
                Primitive(PrimitiveType::CIRCLE, mousePos, 50, 0, { 255, 0, 0, 255 })
            );
        }

        // Rysowanie wszystkich kó³ek
        for (auto& p : primityw) {
            if (p.type == PrimitiveType::CIRCLE) {
                // Jako radius przyjmujemy po³owê p.width
                int radius = p.width / 2;
                Renderer::FillCircle(p.position, radius, p.color);
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderPresent(renderer);
    }
}

void Engine::Shutdown() {
    if (renderer) SDL_DestroyRenderer(renderer);
    if (window) SDL_DestroyWindow(window);
    SDL_Quit();
}
