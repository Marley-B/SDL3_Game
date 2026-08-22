#pragma once
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include "game_object.h"
#include "sdlstate.h" 
#include "game_state.h"
#include "resources.h"

class StaminaUi {
public:
    //Button dimensions
    float kStaminaWidth = 100;
    float kStaminaHeight = 30;

    //Initializes internal variables
    StaminaUi(Resources& res);
    void setPosition(float x, float y); //Sets top left position
    //void setDimensions(float w, float h);
    //bool handleEvent(SDL_Event* e, Resources& res, SDLState& state); //Handles mouse event
    void update(Resources& res, GameState& gs);
    void render(SDLState& state, GameState& gs, GameObject& obj); //Shows button sprite

private:
    //Top left position
    SDL_FPoint mPosition;
    //Currently used sprite
    SDL_Texture* mCurrentTexture;
    SDL_Texture* mBowTexture;

};