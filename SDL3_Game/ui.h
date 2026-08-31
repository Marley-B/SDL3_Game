#pragma once
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include "game_object.h"
#include "sdlstate.h" 
#include "game_state.h"
#include "resources.h"

class Button {
public:
    //Button dimensions
    float kButtonWidth = 100;
    float kButtonHeight = 50;

    //Initializes internal variables
    Button(Resources& res);
    ~Button();
    void setPosition(float x, float y); //Sets top left position
    void setDimensions(float w, float h);
    void setText(SDLState& state, const std::string& text, SDL_Color color);
    bool handleEvent(SDL_Event* e, Resources& res, SDLState& state); //Handles mouse event
    void render(SDLState& state, GameState& gs); //Shows button sprite
    void renderStar(SDLState& state);

private:
    //Top left position
    SDL_FPoint mPosition;
    //Currently used sprite
    SDL_Texture* mCurrentTexture;
    SDL_Texture* mStarTexture;
    SDL_Texture* mText;
};

class StaminaUi {
public:
    //Bar dimensions
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

class CoinUi {
public:
    CoinUi();
    void setPosition(float x, float y); //Sets top left position
    void render(SDLState& state, GameState& gs);
    void renderWithCount(SDLState& state, int count);

private:
    //Top left position
    SDL_FPoint mPosition;
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