#include "Engine.h"
#include "Input.h"
#include "Renderer.h"
#include "Snake.h"
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <algorithm>    // std::min / max          ← NEW

// ───────────────────────── ustawienia okna ───────────────────────────
static constexpr int   WIN_W = 800;
static constexpr int   WIN_H = 600;
static constexpr float ROT_STEP = 5.f;     // stopnie na pojedyncze naciśnięcie
static constexpr float SCALE_UP = 1.10f;   // powiększ
static constexpr float SCALE_DN = 0.90f;   // pomniejsz
// ──────────────────────────────────────────────────────────────────────

Engine::Engine()
    : window(nullptr), renderer(nullptr), isRunning(false),
    isDragging(false), selectedIndex(-1), selectedUnregIndex(-1),
    dragOffset(0, 0), snakeRunning(false) {
}

Engine::~Engine() { Shutdown(); }

bool Engine::Init()
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) {
        std::cerr << "SDL init error: " << SDL_GetError() << '\n';
        return false;
    }

    window = SDL_CreateWindow("Silnik 2D",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WIN_W, WIN_H, SDL_WINDOW_SHOWN);
    if (!window) { std::cerr << "Window error\n"; return false; }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) { std::cerr << "Renderer error\n"; return false; }

    Renderer::Init(renderer);
    std::srand(static_cast<unsigned>(std::time(nullptr)));
    isRunning = true;
    return true;
}

// ─────────────────────────— kolizje pomocnicze —──────────────────────
bool Engine::IsInsideSquare(const Primitive& p, float mx, float my)
{
    return (mx >= p.position.x && mx <= p.position.x + p.width &&
        my >= p.position.y && my <= p.position.y + p.height);
}

bool Engine::IsInsideCircle(const Primitive& p, float mx, float my)
{
    float r = p.width * 0.5f;
    float dx = mx - p.position.x;
    float dy = my - p.position.y;
    return dx * dx + dy * dy <= r * r;
}

bool Engine::IsInsideUnregular(const std::vector<Point2D>& pts, float mx, float my)
{
    if (pts.empty()) return false;
    float minx = pts[0].x, maxx = pts[0].x;
    float miny = pts[0].y, maxy = pts[0].y;
    for (auto& q : pts) {
        minx = std::min(minx, q.x);  maxx = std::max(maxx, q.x);
        miny = std::min(miny, q.y);  maxy = std::max(maxy, q.y);
    }
    return (mx >= minx && mx <= maxx && my >= miny && my <= maxy);
}
// ──────────────────────────────────────────────────────────────────────

void Engine::Run()
{
    SDL_Event ev;

    while (isRunning) {
        // ────────────── obsługa zdarzeń SDL ──────────────
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_QUIT) isRunning = false;
            Input::HandleEvent(ev);

            if (!snakeRunning) {                    // drag-&-drop figur
                if (ev.type == SDL_MOUSEBUTTONDOWN &&
                    ev.button.button == SDL_BUTTON_LEFT)
                {
                    Point2D m = Input::getMausPos();
                    selectedIndex = selectedUnregIndex = -1;

                    // najpierw kwadraty / kółka
                    for (int i = int(primityw.size()) - 1; i >= 0; --i) {
                        auto& sh = primityw[i];
                        if ((sh.type == PrimitiveType::KWADRAT && IsInsideSquare(sh, m.x, m.y)) ||
                            (sh.type == PrimitiveType::CIRCLE && IsInsideCircle(sh, m.x, m.y)))
                        {
                            selectedIndex = i; break;
                        }
                    }
                    // potem wielokąty
                    if (selectedIndex < 0)
                        for (int i = int(unregular.size()) - 1; i >= 0; --i)
                            if (IsInsideUnregular(unregular[i], m.x, m.y))
                            {
                                selectedUnregIndex = i; break;
                            }

                    if (selectedIndex >= 0 || selectedUnregIndex >= 0) {
                        isDragging = true;
                        dragOffset = m;
                    }
                }
                else if (ev.type == SDL_MOUSEBUTTONUP &&
                    ev.button.button == SDL_BUTTON_LEFT)
                {
                    isDragging = false;
                }
                else if (ev.type == SDL_MOUSEMOTION && isDragging) {   // samo przesuwanie
                    Point2D cur = Input::getMausPos();
                    float   dx = cur.x - dragOffset.x;
                    float   dy = cur.y - dragOffset.y;

                    if (selectedIndex >= 0)            // kwadrat lub koło
                        primityw[selectedIndex].position.translate(dx, dy);
                    else if (selectedUnregIndex >= 0)  // wielokąt
                        for (auto& p : unregular[selectedUnregIndex]) p.translate(dx, dy);

                    dragOffset = cur;
                }
            }
        }

        // ────────────── logika silnika ──────────────
        if (Input::IsKeyPressed(SDLK_ESCAPE)) isRunning = false;
        if (!snakeRunning && Input::IsKeyPressed(SDLK_SPACE)) snakeRunning = true;

        // ───── przekształcenia (rotacja / skalowanie) ─────
        if (!snakeRunning && (selectedIndex >= 0 || selectedUnregIndex >= 0))
        {
            int   rotDir = 0;
            if (Input::IsKeyPressed(SDLK_q)) rotDir = -1;
            if (Input::IsKeyPressed(SDLK_e)) rotDir = +1;

            float scale = 1.0f;
            if (Input::IsKeyPressed(SDLK_z)) scale = SCALE_DN;
            if (Input::IsKeyPressed(SDLK_x)) scale = SCALE_UP;

            if (rotDir || scale != 1.0f) {
                // === kwadrat / koło =========================================================
                if (selectedIndex >= 0) {
                    auto& sh = primityw[selectedIndex];

                    if (scale != 1.0f) {
                        if (sh.type == PrimitiveType::KWADRAT) {       // skalowanie względem środka
                            Point2D c{ sh.position.x + sh.width * 0.5f,
                                       sh.position.y + sh.height * 0.5f };
                            sh.width = int(sh.width * scale);
                            sh.height = int(sh.height * scale);
                            sh.position.x = c.x - sh.width * 0.5f;
                            sh.position.y = c.y - sh.height * 0.5f;
                        }
                        else if (sh.type == PrimitiveType::CIRCLE)
                            sh.width = int(sh.width * scale);          // średnica
                    }
                    // (obrót pomijamy – nie ma sensu dla okręgu/kwadratu „z bitmapy”)
                }
                // === wielokąt nieregularny ===================================================
                else {
                    auto& poly = unregular[selectedUnregIndex];

                    // centroid
                    Point2D c{ 0,0 };
                    for (auto& p : poly) { c.x += p.x; c.y += p.y; }
                    c.x /= poly.size();  c.y /= poly.size();

                    if (rotDir)
                        for (auto& p : poly) p = p.rotated(rotDir * ROT_STEP, c);
                    if (scale != 1.0f)
                        for (auto& p : poly) p = p.scaled(scale, scale, c);
                }
            }
        }

        // ────────────── logika gry Snake ──────────────
        if (snakeRunning) {
            snake.HandleInput();
            snake.Update();
            if (!snake.IsAlive()) isRunning = false;
        }

        // ────────────── rendering ──────────────
        SDL_RenderClear(renderer);

        if (!snakeRunning) {
            // tworzenie nowych figur
            if (Input::IsKeyPressed(SDLK_1) && Input::IsMouseButtonPressed(SDL_BUTTON_LEFT))
                primityw.emplace_back(PrimitiveType::KWADRAT, Input::getMausPos(),
                    30, 30, SDL_Color{ 255,0,0,255 });

            if (Input::IsKeyPressed(SDLK_2) && Input::IsMouseButtonPressed(SDL_BUTTON_LEFT)) {
                Point2D m = Input::getMausPos();
                unregular.push_back({ {m.x,m.y}, {m.x + 20,m.y},
                                      {m.x + 30,m.y + 15}, {m.x + 15,m.y + 30},
                                      {m.x,  m.y + 15} });
            }

            if (Input::IsKeyPressed(SDLK_3) && Input::IsMouseButtonPressed(SDL_BUTTON_LEFT))
                primityw.emplace_back(PrimitiveType::CIRCLE, Input::getMausPos(),
                    30, 0, SDL_Color{ 255,0,0,255 });

            // rysuj kwadraty
            for (auto& p : primityw)
                if (p.type == PrimitiveType::KWADRAT)
                    Renderer::FillRect(p.position, p.width, p.height, p.color);

            // rysuj wielokąty
            for (auto& v : unregular) {
                Renderer::UnregularFill(v, SDL_Color{ 255,0,0,255 });
                Renderer::DrawUnregular(v, SDL_Color{ 255,0,0,255 });
            }

            // rysuj kółka
            for (auto& p : primityw)
                if (p.type == PrimitiveType::CIRCLE)
                    Renderer::FillCircle(p.position, p.width / 2, p.color);
        }
        else {
            snake.Render();
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
