#include "Engine.h"
#include "Input.h"
#include "Renderer.h"
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <algorithm>

// ───────── stałe okna & transf. ─────────
static constexpr int   WIN_W = 800;
static constexpr int   WIN_H = 600;
static constexpr float ROT_STEP = 5.f;
static constexpr float SCALE_UP = 1.10f;
static constexpr float SCALE_DN = 0.90f;
static constexpr int   MIN_SIZE = 6;      // nie schodzimy poniżej tego rozmiaru
// ────────────────────────────────────────

Engine::Engine() = default;
Engine::~Engine() { Shutdown(); }

// ──────────────────────────────────────────────────────────────────────
bool Engine::Init()
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
        std::cerr << "SDL init error: " << SDL_GetError() << '\n';
        return false;
    }
    window = SDL_CreateWindow("Silnik 2D",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WIN_W, WIN_H, SDL_WINDOW_SHOWN);
    if (!window) return false;

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) return false;

    Renderer::Init(renderer);
    std::srand(static_cast<unsigned>(std::time(nullptr)));
    isRunning = true;
    return true;
}

// ───────── kolizje pomocnicze ───────────
bool Engine::IsInsideSquare(const Primitive& p, float mx, float my) {
    return mx >= p.position.x && mx <= p.position.x + p.width &&
        my >= p.position.y && my <= p.position.y + p.height;
}
bool Engine::IsInsideCircle(const Primitive& p, float mx, float my) {
    float r = p.width * 0.5f, dx = mx - p.position.x, dy = my - p.position.y;
    return dx * dx + dy * dy <= r * r;
}
bool Engine::IsInsideUnregular(const std::vector<Point2D>& pts, float mx, float my) {
    if (pts.empty())return false;
    float minx = pts[0].x, maxx = pts[0].x, miny = pts[0].y, maxy = pts[0].y;
    for (auto& q : pts) {
        minx = std::min(minx, q.x); maxx = std::max(maxx, q.x);
        miny = std::min(miny, q.y); maxy = std::max(maxy, q.y);
    }
    return mx >= minx && mx <= maxx && my >= miny && my <= maxy;
}
// ────────────────────────────────────────

void Engine::Run()
{
    SDL_Event ev;
    while (isRunning) {
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_QUIT) isRunning = false;
            Input::HandleEvent(ev);

            // ── wybór trybu wstawiania 1/2/3 ──
            if (ev.type == SDL_KEYDOWN && !snakeRunning) {
                switch (ev.key.keysym.sym) {
                case SDLK_1:createMode = CreateMode::KWADRAT;   break;
                case SDLK_2:createMode = CreateMode::UNREGULAR; break;
                case SDLK_3:createMode = CreateMode::CIRCLE;    break;
                }
            }

            // ── przekształcenia – Z/X/Q/E tylko raz na KEYDOWN ──
            if (ev.type == SDL_KEYDOWN && !snakeRunning &&
                (selectedIndex >= 0 || selectedUnregIndex >= 0))
            {
                int rotDir = 0; float scale = 1.f;
                switch (ev.key.keysym.sym) {
                case SDLK_q: rotDir = -1; break;
                case SDLK_e: rotDir = 1; break;
                case SDLK_z: scale = SCALE_DN; break;
                case SDLK_x: scale = SCALE_UP; break;
                default: break;
                }

                if (rotDir || scale != 1.f) {
                    if (selectedIndex >= 0) {
                        auto& sh = primityw[selectedIndex];
                        if (scale != 1.f) {
                            int newD = int(sh.width * scale);
                            if (newD >= MIN_SIZE)
                                sh.width = newD;
                        }
                            /* obrót omiń */
                            // — skalowanie z ROUND i zawsze pozwól powiększać
                            if (scale != 1.f) {
                            int old = sh.width;
                            int n = int(std::round(old * scale));        // ← użyj round()
                            if (scale > 1.f || n >= MIN_SIZE) {
                                sh.width = std::max(n, MIN_SIZE);
                                if (sh.type == PrimitiveType::KWADRAT)
                                     sh.height = std::max(int(std::round(sh.height * scale)), MIN_SIZE); 
                            }
                        }
                            // — obrót Q/E
                            if (rotDir) {
                            sh.angle = std::fmod(sh.angle + rotDir * ROT_STEP + 360.f, 360.f);
                        }
                    }

                    else {                                 // wielokąt
                        auto& poly = unregular[selectedUnregIndex];
                        Point2D c{ 0,0 }; for (auto& p : poly) { c.x += p.x; c.y += p.y; }
                        c.x /= poly.size(); c.y /= poly.size();
                        if (rotDir) for (auto& p : poly)p = p.rotated(rotDir * ROT_STEP, c);
                        if (scale != 1.f) {
                            // sprawdź, czy po skalowaniu figura nie „zniknie”
                            auto test = poly; bool ok = true;
                            for (auto& pt : test) pt = pt.scaled(scale, scale, c);
                            float w = 0, h = 0;
                            float minx = test[0].x, maxx = test[0].x,
                                miny = test[0].y, maxy = test[0].y;
                            for (auto& pt : test) {
                                minx = std::min(minx, pt.x); maxx = std::max(maxx, pt.x);
                                miny = std::min(miny, pt.y); maxy = std::max(maxy, pt.y);
                            }
                            w = maxx - minx; h = maxy - miny;
                            ok = (w >= MIN_SIZE && h >= MIN_SIZE);
                            if (ok) poly.swap(test);
                        }
                    }
                }
            }

            // ── LPM down: tworzenie lub wybór/drag ──
            if (!snakeRunning && ev.type == SDL_MOUSEBUTTONDOWN &&
                ev.button.button == SDL_BUTTON_LEFT)
            {
                Point2D m = Input::getMausPos(); bool created = false;
                if (createMode != CreateMode::NONE) {
                    switch (createMode) {
                    case CreateMode::KWADRAT:
                        primityw.emplace_back(PrimitiveType::KWADRAT, m, 30, 30,
                            SDL_Color{ 255,0,0,255 });
                        selectedIndex = int(primityw.size()) - 1;
                        selectedUnregIndex = -1; break;
                    case CreateMode::CIRCLE:
                        primityw.emplace_back(PrimitiveType::CIRCLE, m, 30, 0,
                            SDL_Color{ 255,0,0,255 });
                        selectedIndex = int(primityw.size()) - 1;
                        selectedUnregIndex = -1; break;
                    case CreateMode::UNREGULAR:
                        unregular.push_back({ {m.x,m.y},{m.x + 20,m.y},
                                             {m.x + 30,m.y + 15},{m.x + 15,m.y + 30},
                                             {m.x,m.y + 15} });
                        selectedUnregIndex = int(unregular.size()) - 1;
                        selectedIndex = -1; break;
                    default:break;
                    }
                    createMode = CreateMode::NONE;
                    isDragging = true; dragOffset = m; created = true;
                }
                if (!created) {
                    selectedIndex = selectedUnregIndex = -1;
                    for (int i = int(primityw.size()) - 1; i >= 0; --i) {
                        auto& sh = primityw[i];
                        if ((sh.type == PrimitiveType::KWADRAT && IsInsideSquare(sh, m.x, m.y)) ||
                            (sh.type == PrimitiveType::CIRCLE && IsInsideCircle(sh, m.x, m.y))) {
                            selectedIndex = i; break;
                        }
                    }
                    if (selectedIndex < 0)
                        for (int i = int(unregular.size()) - 1; i >= 0; --i)
                            if (IsInsideUnregular(unregular[i], m.x, m.y)) {
                                selectedUnregIndex = i; break;
                            }
                    if (selectedIndex >= 0 || selectedUnregIndex >= 0) {
                        isDragging = true; dragOffset = m;
                    }
                }
            }

            // ── LPM up ──
            if (ev.type == SDL_MOUSEBUTTONUP && ev.button.button == SDL_BUTTON_LEFT)
                isDragging = false;

            // ── przeciąganie ──
            if (ev.type == SDL_MOUSEMOTION && isDragging && !snakeRunning) {
                Point2D cur = Input::getMausPos();
                float dx = cur.x - dragOffset.x, dy = cur.y - dragOffset.y;
                if (selectedIndex >= 0)
                    primityw[selectedIndex].position.translate(dx, dy);
                else if (selectedUnregIndex >= 0)
                    for (auto& p : unregular[selectedUnregIndex]) p.translate(dx, dy);
                dragOffset = cur;
            }
        }

        // ── pozostała logika (esc, snake itd.) ──
        if (Input::IsKeyPressed(SDLK_ESCAPE)) isRunning = false;
        if (!snakeRunning && Input::IsKeyPressed(SDLK_SPACE)) snakeRunning = true;

        if (snakeRunning) {
            snake.HandleInput(); snake.Update();
            if (!snake.IsAlive()) isRunning = false;
        }

        // ─────────── RENDER ───────────
        SDL_RenderClear(renderer);
        if (!snakeRunning) {
            for (auto& p : primityw)
                if (p.type == PrimitiveType::KWADRAT)
                    Renderer::FillRectRot(p.position, p.width, p.height, p.angle, p.color);
            for (auto& v : unregular) {
                Renderer::UnregularFill(v, { 255,0,0,255 });
                Renderer::DrawUnregular(v, { 255,0,0,255 });
            }
            for (auto& p : primityw)
                if (p.type == PrimitiveType::CIRCLE)
                    Renderer::FillCircle(p.position, p.width / 2, p.color);
        }
        else snake.Render();

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderPresent(renderer);
    }
}

// ──────────────────────────────────────────────────────────────────────
void Engine::Shutdown()
{
    if (renderer) SDL_DestroyRenderer(renderer);
    if (window)   SDL_DestroyWindow(window);
    SDL_Quit();
}
