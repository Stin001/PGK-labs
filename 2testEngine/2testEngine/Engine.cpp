
#include "Engine.h"
#include "Input.h"
#include "Renderer.h"
#include <iostream>
#include "Primitiv.h"
#include <vector>


Engine::Engine() : window(nullptr), renderer(nullptr), isRunning(false)  {}

Engine::~Engine() {
    Shutdown();
}

bool Engine::Init() {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) {
        std::cerr << "B³¹d inicjalizacji SDL: " << SDL_GetError() << std::endl;
        return false;
    }

    window = SDL_CreateWindow("Silnik 2D", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_SHOWN);
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


        // spawn kwadratow 
        //---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
        if (Input::IsKeyPressed(SDLK_1) && Input::IsMouseButtonPressed(SDL_BUTTON_LEFT)) {



            Point2D mousePos = Input::getMausPos();



            primityw.push_back(Primitive(PrimitiveType::KWADRAT, mousePos, 50, 50, { 255, 0, 0, 255 }));



            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            //SDL_RenderClear(renderer);


            Renderer::FillRect(mousePos, 50, 50, { 255, 0, 0, 255 });

            SDL_RenderPresent(renderer);


        }


        for (auto& p : primityw) {  
            if (p.type == PrimitiveType::KWADRAT) {
                Renderer::FillRect(p.position, p.width, p.height, p.color);
            }
        }

        //---------------------------------------------------------------------------------

        // make unregular cicrale 
        //-------------------------------------------------------------------------------------------------------

        if (Input::IsKeyPressed(SDLK_2) && Input::IsMouseButtonPressed(SDL_BUTTON_LEFT)) {

            Point2D mousePos = Input::getMausPos();  // U¿ywamy mousePos w tym bloku

            std::vector< Point2D > points = {
                {100 + mousePos.x, 100 + mousePos.y},
                {150 + mousePos.x, 50 + mousePos.y},
                {200 + mousePos.x, 100 + mousePos.y},
                {175 + mousePos.x, 150 + mousePos.y},
                {125 + mousePos.x, 150 + mousePos.y},
                {134 + mousePos.x, 168 + mousePos.y}
            };


            

            unregular.push_back(points);


           /* for (const auto& points : unregular) {

                Renderer::UnregularFill(points, { 255, 0, 0, 255 });

                Renderer::DrawUnregular(points, { 255, 0, 0, 255 });
              
              
                
            } */
         }

      

        for (const auto& points : unregular) {

            Renderer::UnregularFill(points, { 255, 0, 0, 255 });
            Renderer::DrawUnregular(points, { 255, 0, 0, 255 });
        }
        


        //---------------------------------------------------------------------


       SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
       




      //  Renderer::DrawPoint(Point2D(400, 300), { 255, 0, 0, 255 });
       // Renderer::DrawLine(Point2D(200, 150), Point2D(600, 450), { 0, 255, 0, 255 });



        SDL_RenderPresent(renderer);
    }
}


void Engine::Shutdown() {
    if (renderer) SDL_DestroyRenderer(renderer);
    if (window) SDL_DestroyWindow(window);
    SDL_Quit();
}