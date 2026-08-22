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
    void update(Resources& res, GameState& gs);
    void render(SDLState& state, GameState& gs, GameObject& obj, Resources& res);

private:
    //Top left position
    SDL_FPoint mPosition;
    //Currently used sprite
    SDL_Texture* mCurrentTexture;
    SDL_Texture* mBoxTexture;
    SDL_Texture* mBgTexture;
};

class TextRenderer {
public:
    TextRenderer(Resources& res);
    SDL_Texture* crateTexture(SDLState& state, const std::string& text, SDL_Color color);
    SDL_Texture* crateTextureVar(SDLState& state, const std::string& text, auto var, SDL_Color color);
    void renderText(SDLState& state, const std::string& text, float x, float y, SDL_Color color,
        bool centered, float centerX, float centerY);
    void renderTextVar(SDLState& state, const std::string& text, auto var, float x, float y, SDL_Color color,
        bool centered, float centerX, float centerY);
    void renderTextTexture(SDL_Texture* textTexture, SDLState& state, float x, float y,
        bool centered, float centerX, float centerY);
private:
    TTF_Font* mFont;
};