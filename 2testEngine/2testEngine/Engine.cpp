#include "Engine.h"
#include "Input.h"
#include "Renderer.h"
#include <iostream>
#include <cmath> // na wszelki wypadek

Engine::Engine()
    : window(nullptr),
    renderer(nullptr),
    isRunning(false),
    isDragging(false),
    selectedIndex(-1),
    selectedUnregIndex(-1),
    dragOffset(0, 0)
{
}

Engine::~Engine() {
    Shutdown();
}

bool Engine::Init() {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) {
        std::cerr << "B³¹d inicjalizacji SDL: " << SDL_GetError() << std::endl;
        return false;
    }

    window = SDL_CreateWindow("Silnik 2D",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
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

// -------------------------------
// Funkcje pomocnicze do wykrywania klikniêcia:

bool Engine::IsInsideSquare(const Primitive& p, float mouseX, float mouseY) {
    float px = p.position.x;
    float py = p.position.y;
    return (mouseX >= px && mouseX <= px + p.width &&
        mouseY >= py && mouseY <= py + p.height);
}

bool Engine::IsInsideCircle(const Primitive& p, float mouseX, float mouseY) {
    float r = p.width / 2.0f;
    float dx = mouseX - p.position.x;
    float dy = mouseY - p.position.y;
    float dist2 = dx * dx + dy * dy;
    return (dist2 <= r * r);
}

bool Engine::IsInsideUnregular(const std::vector<Point2D>& pts, float mouseX, float mouseY) {
    if (pts.empty()) return false;
    float minX = pts[0].x, maxX = pts[0].x;
    float minY = pts[0].y, maxY = pts[0].y;
    for (auto& pt : pts) {
        if (pt.x < minX) minX = pt.x;
        if (pt.x > maxX) maxX = pt.x;
        if (pt.y < minY) minY = pt.y;
        if (pt.y > maxY) maxY = pt.y;
    }
    return (mouseX >= minX && mouseX <= maxX &&
        mouseY >= minY && mouseY <= maxY);
}
// -------------------------------

void Engine::Run() {
    SDL_Event event;
    while (isRunning) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                isRunning = false;
            }
            // obs³uga klawiszy, myszy:
            Input::HandleEvent(event);

            // Obs³uga drag&drop:
            if (event.type == SDL_MOUSEBUTTONDOWN &&
                event.button.button == SDL_BUTTON_LEFT)
            {
                Point2D mPos = Input::getMausPos();
                float mx = mPos.x;
                float my = mPos.y;

                // Szukamy obiektu w primityw (kwadrat/kó³ko) – od koñca
                selectedIndex = -1;
                for (int i = (int)primityw.size() - 1; i >= 0; i--) {
                    Primitive& shape = primityw[i];
                    if (shape.type == PrimitiveType::KWADRAT) {
                        if (IsInsideSquare(shape, mx, my)) {
                            selectedIndex = i;
                            break;
                        }
                    }
                    else if (shape.type == PrimitiveType::CIRCLE) {
                        if (IsInsideCircle(shape, mx, my)) {
                            selectedIndex = i;
                            break;
                        }
                    }
                }

                // Jeœli nie w primityw, to sprawdzamy unregular
                selectedUnregIndex = -1;
                if (selectedIndex < 0) {
                    for (int i = (int)unregular.size() - 1; i >= 0; i--) {
                        if (IsInsideUnregular(unregular[i], mx, my)) {
                            selectedUnregIndex = i;
                            break;
                        }
                    }
                }

                if (selectedIndex >= 0 || selectedUnregIndex >= 0) {
                    isDragging = true;
                    dragOffset = mPos;
                }
            }
            else if (event.type == SDL_MOUSEBUTTONUP &&
                event.button.button == SDL_BUTTON_LEFT)
            {
                isDragging = false;
            }
            else if (event.type == SDL_MOUSEMOTION && isDragging) {
                Point2D currentM = Input::getMausPos();
                float dx = currentM.x - dragOffset.x;
                float dy = currentM.y - dragOffset.y;

                // przesuwamy wybrany obiekt
                if (selectedIndex >= 0) {
                    Primitive& sh = primityw[selectedIndex];
                    if (sh.type == PrimitiveType::KWADRAT ||
                        sh.type == PrimitiveType::CIRCLE)
                    {
                        sh.position.x += dx;
                        sh.position.y += dy;
                    }
                }
                else if (selectedUnregIndex >= 0) {
                    auto& vec = unregular[selectedUnregIndex];
                    for (auto& pt : vec) {
                        pt.x += dx;
                        pt.y += dy;
                    }
                }
                dragOffset = currentM;
            }
        }

        if (Input::IsKeyPressed(SDLK_ESCAPE)) {
            isRunning = false;
        }

        // Wyczyœæ t³o
        SDL_RenderClear(renderer);

        // ---------------------------------
        // Kwadrat ('1') – rozmiar 30x30 zamiast 50x50
        if (Input::IsKeyPressed(SDLK_1) && Input::IsMouseButtonPressed(SDL_BUTTON_LEFT)) {
            Point2D mousePos = Input::getMausPos();
            primityw.push_back(
                Primitive(PrimitiveType::KWADRAT, mousePos, 30, 30, { 255, 0, 0, 255 })
            );
        }

        // Rysowanie wszystkich kwadratów
        for (auto& p : primityw) {
            if (p.type == PrimitiveType::KWADRAT) {
                Renderer::FillRect(p.position, p.width, p.height, p.color);
            }
        }

        // ---------------------------------
        // Nieregularny wielok¹t ('2'), wszystkie przesuniêcia o po³owê
        // (zamiast (40,0)->(20,0), (60,30)->(30,15), (30,60)->(15,30), (0,30)->(0,15))
        if (Input::IsKeyPressed(SDLK_2) && Input::IsMouseButtonPressed(SDL_BUTTON_LEFT)) {
            Point2D mousePos = Input::getMausPos();
            std::vector<Point2D> points = {
                {mousePos.x,      mousePos.y},
                {mousePos.x + 20, mousePos.y},        //  40 -> 20
                {mousePos.x + 30, mousePos.y + 15},   //  60 -> 30; 30->15
                {mousePos.x + 15, mousePos.y + 30},   //  30->15; 60->30
                {mousePos.x,      mousePos.y + 15}    //  30->15
            };
            unregular.push_back(points);
        }

        // Rysowanie wszystkich nieregularnych wielok¹tów
        for (const auto& points : unregular) {
            Renderer::UnregularFill(points, { 255, 0, 0, 255 });
            Renderer::DrawUnregular(points, { 255, 0, 0, 255 });
        }

        // ---------------------------------
        // Kó³ko ('3') – œrednica 30 zamiast 50
        if (Input::IsKeyPressed(SDLK_3) && Input::IsMouseButtonPressed(SDL_BUTTON_LEFT)) {
            Point2D mousePos = Input::getMausPos();
            primityw.push_back(
                Primitive(PrimitiveType::CIRCLE, mousePos, 30, 0, { 255, 0, 0, 255 })
            );
        }

        // Rysowanie wszystkich kó³ek
        for (auto& p : primityw) {
            if (p.type == PrimitiveType::CIRCLE) {
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
