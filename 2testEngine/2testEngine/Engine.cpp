#include "Engine.h"
#include "Input.h"
#include "Renderer.h"
#include <iostream>
#include <cmath>
#include <cstdlib>
#include <ctime>

// ------------------- KONSTANTY PLANSZY -------------------
static constexpr int CELL = 30;         // rozmiar jednej kratki   (was 20)
static constexpr int FRAME = 20;         // gruboœæ szarej ramki
static constexpr int WIDTH = 800;
static constexpr int HEIGHT = 600;
static constexpr int COLS = (WIDTH - 2 * FRAME) / CELL;  // 25 kolumn
static constexpr int ROWS = (HEIGHT - 2 * FRAME) / CELL;  // 18 wierszy
static constexpr int FOOD_R = 12;         // promieñ kó³ka           (was  8/10)
static const SDL_Color COL_A{ 35, 35, 35, 255 }; // kolory pól
static const SDL_Color COL_B{ 60, 60, 60, 255 };
// ---------------------------------------------------------

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
    window = SDL_CreateWindow("Silnik 2D", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WIDTH, HEIGHT, SDL_WINDOW_SHOWN);
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

    srand((unsigned)time(nullptr));
    return true;
}

// ------------------------------------
// Pomocnicze do klikniêcia w kszta³t:
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
    float minX = pts[0].x, maxX = pts[0].x;
    float minY = pts[0].y, maxY = pts[0].y;
    for (auto& pt : pts) {
        if (pt.x < minX) minX = pt.x;
        if (pt.x > maxX) maxX = pt.x;
        if (pt.y < minY) minY = pt.y;
        if (pt.y > maxY) maxY = pt.y;
    }
    return (mx >= minX && mx <= maxX && my >= minY && my <= maxY);
}

// ----------------------------------
// Inicjacja wê¿a
void Engine::InitSnake() {
    snake.clear();
    // centrowanie na planszy
    const float startX = FRAME + (COLS / 2) * CELL;   // 20 + 12·30 = 380
    const float startY = FRAME + (ROWS / 2) * CELL;   // 20 +  9·30 = 290
    snake = { {startX,           startY},
              {startX - CELL,    startY},
              {startX - 2 * CELL,  startY} };

    foodPos.x = FRAME + (rand() % COLS) * CELL + CELL / 2;
    foodPos.y = FRAME + (rand() % ROWS) * CELL + CELL / 2;
}

// ----------------------------------
// Logika wê¿a
void Engine::UpdateSnake() {
    // Spowalniamy wê¿a – co 300 klatek dopiero siê przesuwa
    snakeMoveCounter++;
    if (snakeMoveCounter < 300) return;
    snakeMoveCounter = 0;

    // 1. Przesuwamy ogon
    for (int i = (int)snake.size() - 1; i > 0; i--) {
        snake[i].x = snake[i - 1].x;
        snake[i].y = snake[i - 1].y;
    }
    // G³owa w zale¿noœci od kierunku
    switch (snakeDir) {
    case 0: snake[0].y -= CELL; break; // UP   (-30 px)
    case 1: snake[0].x += CELL; break; // RIGHT(+30 px)
    case 2: snake[0].y += CELL; break; // DOWN (+30 px)
    case 3: snake[0].x -= CELL; break; // LEFT (-30 px)
    }

    // 2. Sprawdzenie kolizji ze œcian¹
    if (snake[0].x < FRAME || snake[0].x >= FRAME + COLS * CELL ||
        snake[0].y < FRAME || snake[0].y >= FRAME + ROWS * CELL) {
        isRunning = false; return;
    }

    // 3. Sprawdzenie kolizji z samym sob¹
    for (size_t i = 1; i < snake.size(); ++i)
        if (snake[0].x == snake[i].x && snake[0].y == snake[i].y) {
            isRunning = false; return;
        }

    // 4. Sprawdzenie zjedzenia kó³ka (kolizja okr¹g-okr¹g)
    float dx = (snake[0].x + CELL / 2) - foodPos.x;
    float dy = (snake[0].y + CELL / 2) - foodPos.y;
    const float sumR = CELL / 2 + FOOD_R;
    if (dx * dx + dy * dy <= sumR * sumR) {
        snake.push_back(snake.back());
        foodPos.x = FRAME + (rand() % COLS) * CELL + CELL / 2;
        foodPos.y = FRAME + (rand() % ROWS) * CELL + CELL / 2;
    }
}

// ----------------------------------
// Rysowanie wê¿a i œcian
void Engine::RenderSnake() {

    // SZACHOWNICA – t³o
    for (int r = 0; r < ROWS; ++r)
        for (int c = 0; c < COLS; ++c) {
            SDL_Color col = ((r + c) & 1) ? COL_A : COL_B;
            Renderer::FillRect({ float(FRAME + c * CELL), float(FRAME + r * CELL) },
                CELL, CELL, col);
        }

    // szara ramka
    Renderer::FillRect({ 0,0 }, WIDTH, FRAME, { 128,128,128,255 });
    Renderer::FillRect({ 0,HEIGHT - FRAME }, WIDTH, FRAME, { 128,128,128,255 });
    Renderer::FillRect({ 0,0 }, FRAME, HEIGHT, { 128,128,128,255 });
    Renderer::FillRect({ WIDTH - FRAME,0 }, FRAME, HEIGHT, { 128,128,128,255 });
    
    // Jedzenie
    Renderer::FillCircle(foodPos, FOOD_R, { 0,255,0,255 });

    // w¹¿
    for (auto& s : snake)
        Renderer::FillRect({ s.x,s.y }, CELL, CELL, { 255,0,0,255 });
}

void Engine::Run() {
    InitSnake();
    SDL_Event event;

    while (isRunning) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                isRunning = false;
            }
            Input::HandleEvent(event);

            // Drag & drop tylko gdy Snake nie wystartowa³
            if (!snakeRunning) {
                if (event.type == SDL_MOUSEBUTTONDOWN &&
                    event.button.button == SDL_BUTTON_LEFT)
                {
                    Point2D mPos = Input::getMausPos();
                    selectedIndex = -1;
                    selectedUnregIndex = -1;
                    for (int i = (int)primityw.size() - 1; i >= 0; i--) {
                        auto& sh = primityw[i];
                        if (sh.type == PrimitiveType::KWADRAT && IsInsideSquare(sh, mPos.x, mPos.y)) {
                            selectedIndex = i; break;
                        }
                        else if (sh.type == PrimitiveType::CIRCLE && IsInsideCircle(sh, mPos.x, mPos.y)) {
                            selectedIndex = i; break;
                        }
                    }
                    if (selectedIndex < 0) {
                        for (int i = (int)unregular.size() - 1; i >= 0; i--) {
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
                else if (event.type == SDL_MOUSEBUTTONUP &&
                    event.button.button == SDL_BUTTON_LEFT)
                {
                    isDragging = false;
                }
                else if (event.type == SDL_MOUSEMOTION && isDragging) {
                    Point2D currentM = Input::getMausPos();
                    float dx = currentM.x - dragOffset.x;
                    float dy = currentM.y - dragOffset.y;
                    if (selectedIndex >= 0) {
                        auto& sh = primityw[selectedIndex];
                        if (sh.type == PrimitiveType::KWADRAT ||
                            sh.type == PrimitiveType::CIRCLE) {
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
        } // end of while(SDL_PollEvent)

        // Wyjœcie ESC
        if (Input::IsKeyPressed(SDLK_ESCAPE)) {
            isRunning = false;
        }
        // Uruchomienie Snake po spacji
        if (Input::IsKeyPressed(SDLK_SPACE) && !snakeRunning) {
            snakeRunning = true;
        }

        // DODANE: blokada ruchu do ty³u
        // ----------------------------------------
        if (snakeRunning) {
            int newDir = snakeDir;
            // Zczytaj wciœniête strza³ki
            if (Input::IsKeyPressed(SDLK_UP))    newDir = 0;
            if (Input::IsKeyPressed(SDLK_RIGHT)) newDir = 1;
            if (Input::IsKeyPressed(SDLK_DOWN))  newDir = 2;
            if (Input::IsKeyPressed(SDLK_LEFT))  newDir = 3;

            // Je¿eli newDir jest "przeciwny" do snakeDir, ignorujemy
            // (0 <-> 2) i (1 <-> 3)
            bool opposite =
                ((snakeDir == 0 && newDir == 2) || (snakeDir == 2 && newDir == 0) ||
                    (snakeDir == 1 && newDir == 3) || (snakeDir == 3 && newDir == 1));
            if (!opposite) {
                snakeDir = newDir;
            }
        }
        // ----------------------------------------

        SDL_RenderClear(renderer);

        if (!snakeRunning) {
            // Rysowanie prymitywów:
            if (Input::IsKeyPressed(SDLK_1) &&
                Input::IsMouseButtonPressed(SDL_BUTTON_LEFT))
            {
                Point2D mp = Input::getMausPos();
                primityw.push_back(
                    Primitive(PrimitiveType::KWADRAT, mp, 30, 30, { 255,0,0,255 }));
            }
            for (auto& p : primityw) {
                if (p.type == PrimitiveType::KWADRAT) {
                    Renderer::FillRect(p.position, p.width, p.height, p.color);
                }
            }
            if (Input::IsKeyPressed(SDLK_2) &&
                Input::IsMouseButtonPressed(SDL_BUTTON_LEFT))
            {
                Point2D mp = Input::getMausPos();
                std::vector<Point2D> v{
                    {mp.x,     mp.y},
                    {mp.x + 20, mp.y},
                    {mp.x + 30, mp.y + 15},
                    {mp.x + 15, mp.y + 30},
                    {mp.x,     mp.y + 15}
                };
                unregular.push_back(v);
            }
            for (const auto& points : unregular) {
                Renderer::UnregularFill(points, { 255,0,0,255 });
                Renderer::DrawUnregular(points, { 255,0,0,255 });
            }
            if (Input::IsKeyPressed(SDLK_3) &&
                Input::IsMouseButtonPressed(SDL_BUTTON_LEFT))
            {
                Point2D mp = Input::getMausPos();
                primityw.push_back(
                    Primitive(PrimitiveType::CIRCLE, mp, 30, 0, { 255,0,0,255 }));
            }
            for (auto& p : primityw) {
                if (p.type == PrimitiveType::CIRCLE) {
                    int r = p.width / 2;
                    Renderer::FillCircle(p.position, r, p.color);
                }
            }
        }
        else {
            // Aktualizacja i rysowanie Snake
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
