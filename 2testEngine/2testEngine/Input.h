#ifndef INPUT_H
#define INPUT_H

#include <SDL.h>
#include "Point2D.h"

class Input {
public:
    static bool IsKeyPressed(SDL_Keycode key);
    static bool IsMouseButtonPressed(Uint8 button);
    static void HandleEvent(const SDL_Event& event);
    static Point2D getMausPos();

private:
    static Uint8 mouseButtons;
    static const Uint8* keyboardState;
    static Point2D MausPos;
};

#endif // INPUT_H
