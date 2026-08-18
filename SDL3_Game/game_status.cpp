// manages the difrent game states the game can change through
#include "game_status.h"

// static IntroState* get();
//IntroState* IntroState::get()
//{
//    //Get static instance
//    return &sIntroState;
//}

bool changeState(GameStatus* newState, GameStatus*& currentState, Resources& res) {
    if (newState != nullptr && newState != currentState) {
        currentState = newState;
        currentState->enter();
        return true;
    }
    return false;
}

// button related functions
Button::Button(Resources& res): 
    mPosition{ 0.f, 0.f }, 
    mCurrentTexture{ res.texButt }
{
}

void Button::setPosition(float x, float y){
    mPosition.x = x;
    mPosition.y = y;
}

void Button::setDimensions(float w, float h) {
    kButtonWidth = w;
    kButtonHeight = h;
}

void Button::handleEvent(SDL_Event* e, Resources& res){
    //If mouse event happened
    if (e->type == SDL_EVENT_MOUSE_MOTION || e->type == SDL_EVENT_MOUSE_BUTTON_DOWN || e->type == SDL_EVENT_MOUSE_BUTTON_UP)
    {
        //Get mouse position
        float x = -1.f, y = -1.f;
        SDL_GetMouseState(&x, &y);

        //Check if mouse is in button
        bool inside = true;

        if (x < mPosition.x){ // left of the button
            inside = false;
        }
        else if (x > mPosition.x + kButtonWidth){ // right of the button
            inside = false;
        }
        else if (y < mPosition.y){ // above the button
            inside = false;
        }
        else if (y > mPosition.y + kButtonHeight){ //  below the button
            inside = false;
        }
        
        if (!inside){ //Mouse is outside button
            mCurrentTexture = res.texButt;
        }
        else{ //Mouse is inside button
            //Set mouse over sprite
            switch (e->type){
                case SDL_EVENT_MOUSE_MOTION:
                    mCurrentTexture = res.texButtHov;
                    break;

                case SDL_EVENT_MOUSE_BUTTON_DOWN:
                    mCurrentTexture = res.texButtDown;
                    break;

                case SDL_EVENT_MOUSE_BUTTON_UP:
                    mCurrentTexture = res.texButtHov;
                    break;
            }
        }
    }
}

void Button::render(SDLState* state) {
    SDL_FRect dst{ // Where on screen to render the sprite
        .x = mPosition.x,
        .y = mPosition.y,
        .w = (kButtonWidth / (float)state->width) * (float)state->logW,
        .h = (kButtonHeight / (float)state->height) * (float)state->logH 
    };
    SDL_RenderTexture(state->renderer, mCurrentTexture, nullptr, &dst);
}