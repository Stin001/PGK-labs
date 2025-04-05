
#ifndef ENGINE_H
#define ENGINE_H
#include "Primitiv.h"
#include <vector>
#include "Input.h"

#define Max_prim  1000
#include <SDL.h>

class Engine {
public:
    Engine();
    ~Engine();

    bool Init();
    void Run();
    void Shutdown();

private:
    bool isRunning;
    SDL_Window* window;
    SDL_Renderer* renderer;
  
    std::vector<Primitive> primityw;
    std::vector<std::vector<Point2D>> unregular;
};

#endif 