#include "Engine.h"
#include "Input.h"
#include "Renderer.h"
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <algorithm>

// ───────── stałe okna & przekształceń ─────────
static constexpr int   WIN_W = 800;
static constexpr int   WIN_H = 600;
static constexpr float ROT_STEP = 5.f;
static constexpr float SCALE_UP = 1.10f;
static constexpr float SCALE_DN = 0.90f;
static constexpr int   MIN_SIZE = 6;
// ──────────────────────────────────────────────

Engine::Engine() = default;
Engine::~Engine() { Shutdown(); }

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

bool Engine::IsInsideSquare(const Primitive& p, float mx, float my)
{
    return mx >= p.position.x && mx <= p.position.x + p.width &&
        my >= p.position.y && my <= p.position.y + p.height;
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
    for (const auto& q : pts) {
        minx = std::min(minx, q.x); maxx = std::max(maxx, q.x);
        miny = std::min(miny, q.y); maxy = std::max(maxy, q.y);
    }
    return mx >= minx && mx <= maxx && my >= miny && my <= maxy;
}

void Engine::Run()
{
    SDL_Event ev;
    while (isRunning)
    {
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_QUIT) isRunning = false;
            Input::HandleEvent(ev);

            // wybór trybu wstawiania 1/2/3/4
            if (ev.type == SDL_KEYDOWN && !snakeRunning) {
                switch (ev.key.keysym.sym) {
                case SDLK_1: createMode = CreateMode::KWADRAT;   break;
                case SDLK_2: createMode = CreateMode::UNREGULAR; break;
                case SDLK_3: createMode = CreateMode::CIRCLE;    break;
                case SDLK_4: createMode = CreateMode::BITMAP;    break;
                default: break;
                }
            }

            // przekształcenia – Q/E/Z/X
            if (ev.type == SDL_KEYDOWN && !snakeRunning &&
                (selectedIndex >= 0 || selectedUnregIndex >= 0))
            {
                int   rotDir = 0;
                float scale = 1.f;
                switch (ev.key.keysym.sym) {
                case SDLK_q: rotDir = -1;         break;
                case SDLK_e: rotDir = 1;         break;
                case SDLK_z: scale = SCALE_DN;   break;
                case SDLK_x: scale = SCALE_UP;   break;
                default: break;
                }

                if (rotDir || scale != 1.f) {
                    // KWADRAT / KOŁO
                    if (selectedIndex >= 0) {
                        auto& sh = primityw[selectedIndex];

                        // obrót Q/E
                        if (rotDir) {
                            sh.angle = std::fmod(sh.angle + rotDir * ROT_STEP + 360.f, 360.f);
                        }
                        // skalowanie Z/X
                        if (scale != 1.f) {
                            int oldW = sh.width;
                            int newW = int(std::round(oldW * scale));
                            if (scale > 1.f || newW >= MIN_SIZE) {
                                sh.width = std::max(newW, MIN_SIZE);
                                if (sh.type == PrimitiveType::KWADRAT)
                                    sh.height = sh.width;
                            }
                        }
                    }
                    else { // wielokąt
                        auto& poly = unregular[selectedUnregIndex];
                        Point2D c{ 0,0 };
                        for (auto& p : poly) { c.x += p.x; c.y += p.y; }
                        c.x /= poly.size(); c.y /= poly.size();

                        if (rotDir)
                            for (auto& p : poly)
                                p = p.rotated(rotDir * ROT_STEP, c);

                        if (scale != 1.f) {
                            auto test = poly;
                            for (auto& p : test) p = p.scaled(scale, scale, c);
                            float minx = test[0].x, maxx = test[0].x;
                            float miny = test[0].y, maxy = test[0].y;
                            for (auto& p : test) {
                                minx = std::min(minx, p.x); maxx = std::max(maxx, p.x);
                                miny = std::min(miny, p.y); maxy = std::max(maxy, p.y);
                            }
                            if (maxx - minx >= MIN_SIZE && maxy - miny >= MIN_SIZE)
                                poly.swap(test);
                        }
                    }
                }
            }

            // LPM down – tworzenie / wybór / drag
            if (!snakeRunning && ev.type == SDL_MOUSEBUTTONDOWN &&
                ev.button.button == SDL_BUTTON_LEFT)
            {
                Point2D m = Input::getMausPos();
                bool created = false;

                if (createMode != CreateMode::NONE) {
                    switch (createMode) {
                    case CreateMode::KWADRAT:
                        primityw.emplace_back(PrimitiveType::KWADRAT, m, 30, 30, SDL_Color{ 255,0,0,255 });
                        selectedIndex = int(primityw.size()) - 1;
                        selectedUnregIndex = -1;
                        break;
                    case CreateMode::CIRCLE:
                        primityw.emplace_back(PrimitiveType::CIRCLE, m, 30, 0, SDL_Color{ 255,0,0,255 });
                        selectedIndex = int(primityw.size()) - 1;
                        selectedUnregIndex = -1;
                        break;
                    case CreateMode::UNREGULAR:
                        unregular.push_back({ {m.x,m.y}, {m.x + 20,m.y}, {m.x + 30,m.y + 15}, {m.x + 15,m.y + 30}, {m.x,m.y + 15} });
                        selectedUnregIndex = int(unregular.size()) - 1;
                        selectedIndex = -1;
                        break;
                    case CreateMode::BITMAP:
                        sprites.emplace_back();
                        {
                            Sprite& sp = sprites.back();
                            if (!sp.bmp.load(renderer, "test.bmp")) {
                                SDL_Log("Nie mogę wczytać test.bmp");
                                sprites.pop_back();
                            }
                            else {
                                sp.pos = m;
                            }
                        }
                        break;
                    default:
                        break;
                    }
                    createMode = CreateMode::NONE;
                    isDragging = true;
                    dragOffset = m;
                    created = true;
                }

                if (!created) {
                    selectedIndex = selectedUnregIndex = -1;
                    for (int i = int(primityw.size()) - 1; i >= 0; --i) {
                        auto& sh = primityw[i];
                        if ((sh.type == PrimitiveType::KWADRAT && IsInsideSquare(sh, m.x, m.y)) ||
                            (sh.type == PrimitiveType::CIRCLE && IsInsideCircle(sh, m.x, m.y))) {
                            selectedIndex = i;
                            break;
                        }
                    }
                    if (selectedIndex < 0) {
                        for (int i = int(unregular.size()) - 1; i >= 0; --i) {
                            if (IsInsideUnregular(unregular[i], m.x, m.y)) {
                                selectedUnregIndex = i;
                                break;
                            }
                        }
                    }
                    if (selectedIndex >= 0 || selectedUnregIndex >= 0) {
                        isDragging = true;
                        dragOffset = m;
                    }
                }
            }

            // LPM up
            if (ev.type == SDL_MOUSEBUTTONUP && ev.button.button == SDL_BUTTON_LEFT)
                isDragging = false;

            // dragging
            if (ev.type == SDL_MOUSEMOTION && isDragging && !snakeRunning) {
                Point2D cur = Input::getMausPos();
                float dx = cur.x - dragOffset.x;
                float dy = cur.y - dragOffset.y;
                if (selectedIndex >= 0)
                    primityw[selectedIndex].position.translate(dx, dy);
                else if (selectedUnregIndex >= 0)
                    for (auto& p : unregular[selectedUnregIndex]) p.translate(dx, dy);
                dragOffset = cur;
            }
        }

        // ESC / SPACE / Snake
        if (Input::IsKeyPressed(SDLK_ESCAPE)) isRunning = false;
        if (!snakeRunning && Input::IsKeyPressed(SDLK_SPACE)) {
            snake.Restart();
            snakeRunning = true;
        }
        if (snakeRunning) {
            snake.HandleInput();
            snake.Update();
            if (!snake.IsAlive())
                snakeRunning = false;
        }

        // RENDERING
        SDL_RenderClear(renderer);

        if (!snakeRunning) {
            // kwadraty
            for (const auto& p : primityw)
                if (p.type == PrimitiveType::KWADRAT)
                    Renderer::FillRectRot(p.position, p.width, p.height, p.angle, p.color);

            // nieregularne wielokąty
            for (const auto& v : unregular) {
                Renderer::UnregularFill(v, { 255,0,0,255 });
                Renderer::DrawUnregular(v, { 255,0,0,255 });
            }

            // kółka
            for (const auto& p : primityw)
                if (p.type == PrimitiveType::CIRCLE)
                    Renderer::FillCircle(p.position, p.width / 2, p.color);

            // bitmapy
            for (const auto& s : sprites)
                s.bmp.render(renderer, int(s.pos.x), int(s.pos.y));
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