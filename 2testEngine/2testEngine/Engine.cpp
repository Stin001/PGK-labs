#include "Engine.h"
#include "Input.h"
#include "Renderer.h"
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <algorithm>

static constexpr int   WIN_W = 800;
static constexpr int   WIN_H = 600;
static constexpr float ROT_STEP = 5.f;
static constexpr float SCALE_UP = 1.10f;
static constexpr float SCALE_DN = 0.90f;
static constexpr int   MIN_SIZE = 6;

Engine::Engine() = default;
Engine::~Engine() { Shutdown(); }

bool Engine::Init() {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
        std::cerr << "SDL init error: " << SDL_GetError() << "\n";
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

void Engine::Run() {
    SDL_Event ev;
    while (isRunning) {
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_QUIT) isRunning = false;
            Input::HandleEvent(ev);
            HandleInput(ev);
        }

        // wyjście i uruchomienie Snake’a
        if (Input::IsKeyPressed(SDLK_ESCAPE)) isRunning = false;
        if (!snakeRunning && Input::IsKeyPressed(SDLK_SPACE)) {
            snake.Restart();
            snakeRunning = true;
        }

        // Snake vs. edytor
        if (snakeRunning) {
            snake.HandleInput();
            snake.Update();
            if (!snake.IsAlive()) snakeRunning = false;
        }

        // animacja sprite’ów
        updateBitmapObjects();

        // rysowanie
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        if (!snakeRunning) {
            // prymitywy
            for (const auto& p : primityw) {
                if (p.type == PrimitiveType::KWADRAT)
                    Renderer::FillRectRot(p.position, p.width, p.height, p.angle, p.color);
                else if (p.type == PrimitiveType::CIRCLE)
                    Renderer::FillCircle(p.position, p.width / 2, p.color);
            }
            for (const auto& v : unregular) {
                Renderer::UnregularFill(v, { 255,0,0,255 });
                Renderer::DrawUnregular(v, { 255,0,0,255 });
            }
            // sprite’y
            renderBitmapObjects();
        }
        else {
            snake.Render();
        }

        SDL_RenderPresent(renderer);
    }
}

void Engine::HandleInput(const SDL_Event& ev) {
    if (snakeRunning) return;

    if (ev.type == SDL_KEYDOWN) {
        switch (ev.key.keysym.sym) {
        case SDLK_1: createMode = CreateMode::KWADRAT;   break;
        case SDLK_2: createMode = CreateMode::UNREGULAR; break;
        case SDLK_3: createMode = CreateMode::CIRCLE;    break;
        case SDLK_4: createMode = CreateMode::BITMAP;    break;
        case SDLK_SPACE:
            for (auto& bmp : bitmapObjects)
                if (!bmp.isAnimating)
                    bmp.startAnimation({ 5,0 });
            break;
        case SDLK_q: case SDLK_e: case SDLK_z: case SDLK_x:
            // obrót/skalowanie zaznaczonego obiektu
            if (selectedIndex >= 0 || selectedUnregIndex >= 0) {
                int rotDir = (ev.key.keysym.sym == SDLK_q ? -1 : (ev.key.keysym.sym == SDLK_e ? 1 : 0));
                float scale = (ev.key.keysym.sym == SDLK_z ? SCALE_DN : (ev.key.keysym.sym == SDLK_x ? SCALE_UP : 1.f));
                if (rotDir != 0 || scale != 1.f) {
                    if (selectedIndex >= 0) {
                        auto& sh = primityw[selectedIndex];
                        if (rotDir) sh.angle = std::fmod(sh.angle + rotDir * ROT_STEP + 360.f, 360.f);
                        if (scale != 1.f) {
                            int oldW = sh.width;
                            int newW = int(std::round(oldW * scale));
                            if (scale > 1.f || newW >= MIN_SIZE) {
                                sh.width = std::max(newW, MIN_SIZE);
                                if (sh.type == PrimitiveType::KWADRAT) sh.height = sh.width;
                            }
                        }
                    }
                    else {
                        auto& poly = unregular[selectedUnregIndex];
                        Point2D c{ 0,0 };
                        for (auto& p : poly) { c.x += p.x; c.y += p.y; }
                        c.x /= poly.size(); c.y /= poly.size();
                        if (rotDir)
                            for (auto& p : poly) p = p.rotated(rotDir * ROT_STEP, c);
                        if (scale != 1.f) {
                            auto test = poly;
                            for (auto& p : test) p = p.scaled(scale, scale, c);
                            float minx = test[0].x, maxx = test[0].x, miny = test[0].y, maxy = test[0].y;
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
            break;
        default: break;
        }
    }

    // myszka: tworzenie / wybór / drag
    if (ev.type == SDL_MOUSEBUTTONDOWN && ev.button.button == SDL_BUTTON_LEFT) {
        Point2D m = Input::getMausPos();
        bool created = false;
        if (createMode != CreateMode::NONE) {
            switch (createMode) {
            case CreateMode::KWADRAT:
                primityw.emplace_back(PrimitiveType::KWADRAT, m, 30, 30, { 255,0,0,255 });
                selectedIndex = int(primityw.size()) - 1;
                selectedUnregIndex = -1;
                created = true;
                break;
            case CreateMode::CIRCLE:
                primityw.emplace_back(PrimitiveType::CIRCLE, m, 30, 0, { 255,0,0,255 });
                selectedIndex = int(primityw.size()) - 1;
                selectedUnregIndex = -1;
                created = true;
                break;
            case CreateMode::UNREGULAR:
                unregular.push_back({ {m.x,m.y},{m.x + 20,m.y},{m.x + 30,m.y + 15},{m.x + 15,m.y + 30},{m.x,m.y + 15} });
                selectedUnregIndex = int(unregular.size()) - 1;
                selectedIndex = -1;
                created = true;
                break;
            case CreateMode::BITMAP: {
                SpriteObject sp;
                if (sp.load(renderer, "test.bmp", m, 2, 32, 32))
                    bitmapObjects.push_back(sp);
                created = true;
                break;
            }
            default: break;
            }
            createMode = CreateMode::NONE;
            isDragging = true;
            dragOffset = m;
        }
        if (!created) {
            selectedIndex = selectedUnregIndex = -1;
            for (int i = int(primityw.size()) - 1; i >= 0; --i) {
                auto& p = primityw[i];
                if ((p.type == PrimitiveType::KWADRAT && IsInsideSquare(p, m.x, m.y)) ||
                    (p.type == PrimitiveType::CIRCLE && IsInsideCircle(p, m.x, m.y))) {
                    selectedIndex = i; break;
                }
            }
            if (selectedIndex < 0) {
                for (int i = int(unregular.size()) - 1; i >= 0; --i) {
                    if (IsInsideUnregular(unregular[i], m.x, m.y)) {
                        selectedUnregIndex = i; break;
                    }
                }
            }
            if (selectedIndex >= 0 || selectedUnregIndex >= 0) {
                isDragging = true;
                dragOffset = m;
            }
        }
    }

    if (ev.type == SDL_MOUSEBUTTONUP && ev.button.button == SDL_BUTTON_LEFT)
        isDragging = false;

    if (ev.type == SDL_MOUSEMOTION && isDragging) {
        Point2D cur = Input::getMausPos();
        float dx = cur.x - dragOffset.x, dy = cur.y - dragOffset.y;
        if (selectedIndex >= 0)
            primityw[selectedIndex].position.translate(dx, dy);
        else if (selectedUnregIndex >= 0)
            for (auto& pt : unregular[selectedUnregIndex]) pt.translate(dx, dy);
        dragOffset = cur;
    }
}

void Engine::addBitmapObject(const std::string& path, const Point2D& position) {
    SpriteObject sp;
    if (sp.load(renderer, path, position, 2, 32, 32))
        bitmapObjects.push_back(sp);
}

void Engine::updateBitmapObjects() {
    for (auto& obj : bitmapObjects) obj.animate();
}

void Engine::renderBitmapObjects() {
    for (auto& obj : bitmapObjects) obj.render(renderer);
}

void Engine::Shutdown() {
    if (renderer) SDL_DestroyRenderer(renderer);
    if (window)   SDL_DestroyWindow(window);
    SDL_Quit();
}
