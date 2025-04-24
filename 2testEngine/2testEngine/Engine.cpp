#include "Engine.h"
#include "Input.h"
#include "Renderer.h"
#include "Snake.h"           // << korzystamy z gry w osobnym pliku
#include <iostream>
#include <cstdlib>
#include <ctime>

// ───────────────────────── PARAMETRY OKNA ────────────────────────────
static constexpr int WIN_W = 800;
static constexpr int WIN_H = 600;
// ──────────────────────────────────────────────────────────────────────

Engine::Engine()
    : window(nullptr), renderer(nullptr), isRunning(false),
    isDragging(false), selectedIndex(-1), selectedUnregIndex(-1),
    dragOffset(0, 0),
    snakeRunning(false)                         // tylko flaga uruchomienia
{
}

Engine::~Engine() { Shutdown(); }

bool Engine::Init()
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
        std::cerr << "SDL init error: " << SDL_GetError() << std::endl;
        return false;
    }
    window = SDL_CreateWindow("Silnik 2D",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WIN_W, WIN_H, SDL_WINDOW_SHOWN);
    if (!window) { std::cerr << "Window error\n"; return false; }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) { std::cerr << "Renderer error\n"; return false; }

    Renderer::Init(renderer);
    std::srand((unsigned)std::time(nullptr));
    isRunning = true;
    return true;
}

// ───────────────────── pomocnicze testy kolizji do drag&drop ─────────
bool Engine::IsInsideSquare(const Primitive& p, float mx, float my) {
    return (mx >= p.position.x && mx <= p.position.x + p.width &&
        my >= p.position.y && my <= p.position.y + p.height);
}
bool Engine::IsInsideCircle(const Primitive& p, float mx, float my) {
    float r = p.width / 2.0f, dx = mx - p.position.x, dy = my - p.position.y;
    return dx * dx + dy * dy <= r * r;
}
bool Engine::IsInsideUnregular(const std::vector<Point2D>& pts, float mx, float my) {
    if (pts.empty()) return false;
    float minx = pts[0].x, maxx = pts[0].x, miny = pts[0].y, maxy = pts[0].y;
    for (const auto& q : pts) {
        if (q.x < minx)minx = q.x; if (q.x > maxx)maxx = q.x;
        if (q.y < miny)miny = q.y; if (q.y > maxy)maxy = q.y;
    }
    return (mx >= minx && mx <= maxx && my >= miny && my <= maxy);
}
// ──────────────────────────────────────────────────────────────────────

void Engine::Run()
{
    SDL_Event ev;

    while (isRunning) {
        // ========== Zdarzenia SDL ==========
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_QUIT) isRunning = false;
            Input::HandleEvent(ev);

            // ─── drag & drop figur zanim wystartuje Snake ───
            if (!snakeRunning) {
                if (ev.type == SDL_MOUSEBUTTONDOWN && ev.button.button == SDL_BUTTON_LEFT) {
                    Point2D m = Input::getMausPos();
                    selectedIndex = selectedUnregIndex = -1;

                    // sprawdź kwadraty / kółka
                    for (int i = int(primityw.size()) - 1; i >= 0; --i) {
                        auto& sh = primityw[i];
                        if (sh.type == PrimitiveType::KWADRAT && IsInsideSquare(sh, m.x, m.y)) {
                            selectedIndex = i; break;
                        }
                        else
                            if (sh.type == PrimitiveType::CIRCLE && IsInsideCircle(sh, m.x, m.y)) {
                                selectedIndex = i; break;
                            }
                    }
                    // sprawdź wielokąty nieregularne
                    if (selectedIndex < 0) {
                        for (int i = int(unregular.size()) - 1; i >= 0; --i) {
                            if (IsInsideUnregular(unregular[i], m.x, m.y)) {
                                selectedUnregIndex = i; break;
                            }
                        }
                    }
                    if (selectedIndex >= 0 || selectedUnregIndex >= 0) {
                        isDragging = true; dragOffset = m;
                    }
                }
                else if (ev.type == SDL_MOUSEBUTTONUP && ev.button.button == SDL_BUTTON_LEFT) {
                    isDragging = false;
                }
                else if (ev.type == SDL_MOUSEMOTION && isDragging) {
                    Point2D cur = Input::getMausPos();
                    float dx = cur.x - dragOffset.x;
                    float dy = cur.y - dragOffset.y;

                    if (selectedIndex >= 0) {
                        auto& sh = primityw[selectedIndex];
                        sh.position.x += dx; sh.position.y += dy;
                    }
                    else if (selectedUnregIndex >= 0) {
                        for (auto& pt : unregular[selectedUnregIndex]) {
                            pt.x += dx; pt.y += dy;
                        }
                    }
                    dragOffset = cur;
                }
            }
            // ────────────────────────────────────────────────
        }

        // ========== logika silnika ==========
        if (Input::IsKeyPressed(SDLK_ESCAPE)) isRunning = false;
        if (Input::IsKeyPressed(SDLK_SPACE) && !snakeRunning) {
            snakeRunning = true;   // start gry
        }

        // Sterowanie wężem i logika gry
        if (snakeRunning) {
            snake.HandleInput();   // strzałki + blokada zawracania
            snake.Update();        // ruch, kolizje, jedzenie
            if (!snake.IsAlive()) isRunning = false; // zakończ program po kolizji
        }

        // ========== Rendering ==========
        SDL_RenderClear(renderer);

        if (!snakeRunning) {
            // ---------------- rysowanie prymitywów -------------
            if (Input::IsKeyPressed(SDLK_1) && Input::IsMouseButtonPressed(SDL_BUTTON_LEFT)) {
                Point2D m = Input::getMausPos();
                primityw.push_back(Primitive(PrimitiveType::KWADRAT, m, 30, 30, { 255,0,0,255 }));
            }
            for (const auto& p : primityw)
                if (p.type == PrimitiveType::KWADRAT)
                    Renderer::FillRect(p.position, p.width, p.height, p.color);

            if (Input::IsKeyPressed(SDLK_2) && Input::IsMouseButtonPressed(SDL_BUTTON_LEFT)) {
                Point2D m = Input::getMausPos();
                unregular.push_back({ {m.x,m.y},{m.x + 20,m.y},{m.x + 30,m.y + 15},
                                      {m.x + 15,m.y + 30},{m.x,m.y + 15} });
            }
            for (const auto& v : unregular) {
                Renderer::UnregularFill(v, { 255,0,0,255 });
                Renderer::DrawUnregular(v, { 255,0,0,255 });
            }

            if (Input::IsKeyPressed(SDLK_3) && Input::IsMouseButtonPressed(SDL_BUTTON_LEFT)) {
                Point2D m = Input::getMausPos();
                primityw.push_back(Primitive(PrimitiveType::CIRCLE, m, 30, 0, { 255,0,0,255 }));
            }
            for (const auto& p : primityw)
                if (p.type == PrimitiveType::CIRCLE)
                    Renderer::FillCircle(p.position, p.width / 2, p.color);
            // ----------------------------------------------------
        }
        else {
            snake.Render();        // rysuje tło-szachownicę, węża i kółko
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderPresent(renderer);
    }
}

void Engine::Shutdown()
{
    if (renderer) SDL_DestroyRenderer(renderer);
    if (window)   SDL_DestroyWindow(window);
    SDL_Quit();
}
