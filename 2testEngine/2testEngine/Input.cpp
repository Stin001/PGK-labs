
#include "Input.h"

Uint8 Input::mouseButtons = 0;
const Uint8* Input::keyboardState = SDL_GetKeyboardState(nullptr);
Point2D Input::MausPos = Point2D(0, 0);

bool Input::IsKeyPressed(SDL_Keycode key) {
    SDL_Scancode scancode = SDL_GetScancodeFromKey(key);
    return keyboardState[scancode];
}

bool Input::IsMouseButtonPressed(Uint8 button) {
    return mouseButtons & SDL_BUTTON(button);
}

void Input::HandleEvent(const SDL_Event& event) {
    if (event.type == SDL_MOUSEBUTTONDOWN) {
        mouseButtons |= SDL_BUTTON(event.button.button);
          MausPos = Point2D(static_cast<float>(event.button.x), static_cast<float>(event.button.y));  // Zapisujemy pozycjê myszy
    }
    else if (event.type == SDL_MOUSEBUTTONUP) {
        mouseButtons &= ~SDL_BUTTON(event.button.button);
    }

}



Point2D Input::getMausPos()
{

    return MausPos;
}

