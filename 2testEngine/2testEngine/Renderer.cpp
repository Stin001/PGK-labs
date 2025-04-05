
#include "Renderer.h"
#include <SDL.h>

#include <iostream>

SDL_Renderer* Renderer::sdlRenderer = nullptr;

void Renderer::Init(SDL_Renderer* renderer) {
    sdlRenderer = renderer;
}

void Renderer::DrawPoint(const Point2D& point, SDL_Color color) {
    SDL_SetRenderDrawColor(sdlRenderer, color.r, color.g, color.b, color.a);
    SDL_RenderDrawPoint(sdlRenderer, static_cast<int>(point.x), static_cast<int>(point.y));
}

void Renderer::DrawLine(const Point2D& start, const Point2D& end, SDL_Color color) {
    SDL_SetRenderDrawColor(sdlRenderer, color.r, color.g, color.b, color.a);
    SDL_RenderDrawLine(sdlRenderer, static_cast<int>(start.x), static_cast<int>(start.y),
        static_cast<int>(end.x), static_cast<int>(end.y));
}

void Renderer::DrawRect(const Point2D& position, int width, int height, SDL_Color color)
{
    SDL_SetRenderDrawColor(sdlRenderer, color.r, color.g, color.b, color.a);
    SDL_Rect rect = { static_cast<int>(position.x), static_cast<int>(position.y), width, height };
    SDL_RenderDrawRect(sdlRenderer, &rect);
}

void Renderer::FillRect(const Point2D& position, int width, int height, SDL_Color color)
{
    SDL_SetRenderDrawColor(sdlRenderer, color.r, color.g, color.b, color.a);
    SDL_Rect rect = { static_cast<int>(position.x), static_cast<int>(position.y), width, height };
    SDL_RenderFillRect(sdlRenderer, &rect);

}

void Renderer::DrawUnregular(const std::vector<Point2D>& points, SDL_Color color)
{
    if (points.size() < 2) return;


    SDL_SetRenderDrawColor(sdlRenderer, color.r, color.g, color.b, color.a);


    for (size_t i = 0; i < points.size() - 1; ++i) {
        SDL_RenderDrawLine(sdlRenderer, static_cast<int>(points[i].x), static_cast<int>(points[i].y),
            static_cast<int>(points[i + 1].x), static_cast<int>(points[i + 1].y));
    }


    SDL_RenderDrawLine(sdlRenderer, static_cast<int>(points[points.size() - 1].x),
        static_cast<int>(points[points.size() - 1].y),
        static_cast<int>(points[0].x), static_cast<int>(points[0].y));
};




void Renderer::UnregularFill(const std::vector<Point2D>& points, const SDL_Color& color) {

    SDL_SetRenderDrawColor(sdlRenderer, color.r, color.g, color.b, color.a);


    if (points.size() < 3) return; 



    int minY = points[0].y;
    int maxY = points[0].y;



    for (const auto& p : points) {
        if (p.y < minY) minY = p.y;
        if (p.y > maxY) maxY = p.y;
    }


//    if (maxY - minY > 600) {  
  //      maxY = minY + (maxY - minY);
   // }



    int minX = points[0].x;
    int maxX = points[0].x;

    for (const auto& p : points) {
        if (p.x < minX) minX = p.x;
        if (p.x > maxX) maxX = p.x;
    }

    
   // if (maxX - minX > 800) {
     //   maxX = minX + (maxX - minX);
    //}



     //std::cout << "minY: " << minY << ", maxY: " << maxY << std::endl;
    //std::cout << "minX: " << minX << ", maxX: " << maxX << std::endl;



    for (int y = minY; y <= maxY; y++) {

        std::vector<int> partycions;



        for (size_t i = 0; i < points.size(); i++) {


            size_t j = (i + 1) % points.size();

            int y1 = points[i].y, y2 = points[j].y;
            int x1 = points[i].x, x2 = points[j].x;

            if ((y1 < y && y2 >= y) || (y2 < y && y1 >= y)) {
                int x = 0;
                if (y1 != y2) {
                    
                    x = static_cast<int>(x1 + (y - y1) * (x2 - x1) / static_cast<float>(y2 - y1));

                   
                }
                partycions.push_back(x);

            }
        }


        std::sort(partycions.begin(), partycions.end());


        for (size_t i = 0; i + 1 < partycions.size(); i += 2) {
            if (i + 1 < partycions.size()) {

                if (partycions[i] < minX) partycions[i] = minX;
                if (partycions[i + 1] > maxX) partycions[i + 1] = maxX;


                SDL_RenderDrawLine(sdlRenderer, partycions[i], y, partycions[i + 1], y);
            }



        }





    }






}