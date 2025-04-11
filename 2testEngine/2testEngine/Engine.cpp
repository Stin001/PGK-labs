#include "Engine.h"
#include "Input.h"
#include "Renderer.h"
#include <iostream>
#include <cmath>
#include <cstdlib>  // rand, srand
#include <ctime>    // time

Engine::Engine()
    : window(nullptr), renderer(nullptr), isRunning(false),
    isDragging(false), selectedIndex(-1), selectedUnregIndex(-1),
    dragOffset(0, 0),
    snakeRunning(false),
    snakeDir(1),
    snakeMoveCounter(0)
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

    srand((unsigned)time(nullptr)); // do losowania jedzenia wê¿a
    return true;
}

// ---------------------
// Pomocnicze do klikniêcia w kszta³t (bez zmian)
bool Engine::IsInsideSquare(const Primitive& p, float mouseX, float mouseY) {
    float px = p.position.x, py = p.position.y;
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
bool Engine::IsInsideUnregular(const std::vector<Point2D>& pts, float mx, float my) {
    if (pts.empty()) return false;
    float minX = pts[0].x, maxX = pts[0].x, minY = pts[0].y, maxY = pts[0].y;
    for (auto& pt : pts) {
        if (pt.x < minX)minX = pt.x; if (pt.x > maxX)maxX = pt.x;
        if (pt.y < minY)minY = pt.y; if (pt.y > maxY)maxY = pt.y;
    }
    return (mx >= minX && mx <= maxX && my >= minY && my <= maxY);
}
// ---------------------

// ---------------------
// Inicjacja wê¿a
void Engine::InitSnake() {
    snake.clear();
    // 3 segmenty wê¿a
    snake.push_back({ 400.0f,300.0f });
    snake.push_back({ 380.0f,300.0f });
    snake.push_back({ 360.0f,300.0f });
    snakeDir = 1; // startowo w prawo

    // Losujemy jedzenie co 20 pikseli
    foodPos.x = float((rand() % 39) * 20);
    foodPos.y = float((rand() % 29) * 20);
}

// ---------------------
// Logika wê¿a
void Engine::UpdateSnake() {
    // ZMIENIONE: spowolnione z 10 do 20 klatek
    snakeMoveCounter++;
    if (snakeMoveCounter < 280) return;
    snakeMoveCounter = 0;

    // przesuwamy ogon
    for (int i = int(snake.size()) - 1; i > 0; i--) {
        snake[i].x = snake[i - 1].x;
        snake[i].y = snake[i - 1].y;
    }
    // przesuniêcie g³owy
    switch (snakeDir) {
    case 0: snake[0].y -= 20; break; // UP
    case 1: snake[0].x += 20; break; // RIGHT
    case 2: snake[0].y += 20; break; // DOWN
    case 3: snake[0].x -= 20; break; // LEFT
    }

    // Sprawdzamy uderzenie w œcianê (okno ma 800x600):
    if (snake[0].x < 0 || snake[0].x >= 800 ||
        snake[0].y < 0 || snake[0].y >= 600) {
        // koniec gry = koniec programu
        isRunning = false;
        return;
    }

    // Zbieranie jedzenia
    if (std::fabs(snake[0].x - foodPos.x) < 1e-1 &&
        std::fabs(snake[0].y - foodPos.y) < 1e-1) {
        // dodaj segment
        snake.push_back({ snake.back().x,snake.back().y });
        // losuj nowe jedzenie
        foodPos.x = float((rand() % 39) * 20);
        foodPos.y = float((rand() % 29) * 20);
    }
}

// ---------------------
// Rysowanie wê¿a i œcian
void Engine::RenderSnake() {
    // Rysujemy œciany wokó³ planszy jako szare prostok¹ty
    // górna
    Renderer::FillRect({ 0,0 }, 800, 20, { 128,128,128,255 });
    // dolna
    Renderer::FillRect({ 0,580 }, 800, 20, { 128,128,128,255 });
    // lewa
    Renderer::FillRect({ 0,0 }, 20, 600, { 128,128,128,255 });
    // prawa
    Renderer::FillRect({ 780,0 }, 20, 600, { 128,128,128,255 });

    // Jedzenie - zielone kó³ko (promieñ=10)
    Renderer::FillCircle(foodPos, 10, { 0,255,0,255 });

    // W¹¿ – czerwone kwadraty 20×20
    for (auto& seg : snake) {
        Renderer::FillRect({ seg.x, seg.y }, 20, 20, { 255,0,0,255 });
    }
}
// ---------------------

void Engine::Run() {
    InitSnake(); // przygotowanie wê¿a od razu
    SDL_Event event;
    while (isRunning) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                isRunning = false;
            }
            Input::HandleEvent(event);

            // Drag & drop tylko gdy nie uruchomiliœmy Snake
            if (!snakeRunning) {
                if (event.type == SDL_MOUSEBUTTONDOWN &&
                    event.button.button == SDL_BUTTON_LEFT)
                {
                    Point2D mPos = Input::getMausPos();
                    selectedIndex = -1; selectedUnregIndex = -1;
                    for (int i = int(primityw.size()) - 1; i >= 0; i--) {
                        auto& sh = primityw[i];
                        if (sh.type == PrimitiveType::KWADRAT && IsInsideSquare(sh, mPos.x, mPos.y)) {
                            selectedIndex = i; break;
                        }
                        else if (sh.type == PrimitiveType::CIRCLE && IsInsideCircle(sh, mPos.x, mPos.y)) {
                            selectedIndex = i; break;
                        }
                    }
                    if (selectedIndex < 0) {
                        for (int i = int(unregular.size()) - 1; i >= 0; i--) {
                            if (IsInsideUnregular(unregular[i], mPos.x, mPos.y)) {
                                selectedUnregIndex = i; break;
                            }
                        }
                    }
                    if (selectedIndex >= 0 || selectedUnregIndex >= 0) {
                        isDragging = true;
                        dragOffset = mPos;
                    }
                }
                else if (event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_LEFT) {
                    isDragging = false;
                }
                else if (event.type == SDL_MOUSEMOTION && isDragging) {
                    Point2D currentM = Input::getMausPos();
                    float dx = currentM.x - dragOffset.x;
                    float dy = currentM.y - dragOffset.y;
                    if (selectedIndex >= 0) {
                        auto& sh = primityw[selectedIndex];
                        if (sh.type == PrimitiveType::KWADRAT || sh.type == PrimitiveType::CIRCLE) {
                            sh.position.x += dx; sh.position.y += dy;
                        }
                    }
                    else if (selectedUnregIndex >= 0) {
                        auto& vec = unregular[selectedUnregIndex];
                        for (auto& pt : vec) { pt.x += dx; pt.y += dy; }
                    }
                    dragOffset = currentM;
                }
            }
        } // koniec SDL_PollEvent

        // Wyjœcie ESC
        if (Input::IsKeyPressed(SDLK_ESCAPE)) {
            isRunning = false;
        }
        // Uruchomienie Snake po spacji
        if (Input::IsKeyPressed(SDLK_SPACE) && !snakeRunning) {
            snakeRunning = true;
        }
        // Sterowanie wê¿em strza³kami
        if (snakeRunning) {
            if (Input::IsKeyPressed(SDLK_UP))    snakeDir = 0;
            if (Input::IsKeyPressed(SDLK_RIGHT)) snakeDir = 1;
            if (Input::IsKeyPressed(SDLK_DOWN))  snakeDir = 2;
            if (Input::IsKeyPressed(SDLK_LEFT))  snakeDir = 3;
        }

        SDL_RenderClear(renderer);

        if (!snakeRunning) {
            // Rysowanie prymitywów
            if (Input::IsKeyPressed(SDLK_1) && Input::IsMouseButtonPressed(SDL_BUTTON_LEFT)) {
                Point2D mp = Input::getMausPos();
                primityw.push_back(Primitive(PrimitiveType::KWADRAT, mp, 30, 30, { 255,0,0,255 }));
            }
            for (auto& p : primityw) {
                if (p.type == PrimitiveType::KWADRAT) {
                    Renderer::FillRect(p.position, p.width, p.height, p.color);
                }
            }
            if (Input::IsKeyPressed(SDLK_2) && Input::IsMouseButtonPressed(SDL_BUTTON_LEFT)) {
                Point2D mp = Input::getMausPos();
                std::vector<Point2D> v{
                    {mp.x,mp.y},{mp.x + 20,mp.y},
                    {mp.x + 30,mp.y + 15},{mp.x + 15,mp.y + 30},{mp.x,mp.y + 15}
                };
                unregular.push_back(v);
            }
            for (const auto& points : unregular) {
                Renderer::UnregularFill(points, { 255,0,0,255 });
                Renderer::DrawUnregular(points, { 255,0,0,255 });
            }
            if (Input::IsKeyPressed(SDLK_3) && Input::IsMouseButtonPressed(SDL_BUTTON_LEFT)) {
                Point2D mp = Input::getMausPos();
                primityw.push_back(Primitive(PrimitiveType::CIRCLE, mp, 30, 0, { 255,0,0,255 }));
            }
            for (auto& p : primityw) {
                if (p.type == PrimitiveType::CIRCLE) {
                    int r = p.width / 2;
                    Renderer::FillCircle(p.position, r, p.color);
                }
            }
        }
        else {
            // Obs³uga Snake
            UpdateSnake();
            RenderSnake();
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
