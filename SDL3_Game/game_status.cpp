// manages the difrent game states the game can change through
#include "game_status.h"


IntroMenu* IntroMenu::get(){
    return &sIntroMenu;
}

bool IntroMenu::enter(Resources& res) {
    mBgTexture = res.texIntroScreen;
    return true;
}

void IntroMenu::handleEvent(SDL_Event& e, GameState& gs, Resources& res) {
    if ((e.type == SDL_EVENT_KEY_DOWN) && ((e.key.key == SDLK_KP_ENTER) || (e.key.key == SDLK_RETURN))) {
        changeState(LevelMenu::get(), gs.currentStateGame , res);
    }
}

void IntroMenu::update() {

}

void IntroMenu::render(SDLState* state) {
    SDL_RenderTexture(state->renderer, mBgTexture, nullptr, nullptr);
}

LevelMenu* LevelMenu::get() {
    return &sLevelMenu;
}

bool LevelMenu::enter(Resources& res) {
    mBgTexture = res.texLevelMenuScreen;
    button1 = Button(res);
    button1.setPosition(20, 150);
    button2 = Button(res);
    button2.setPosition(100, 150);
    button3 = Button(res);
    button3.setPosition(200, 150);
    button4 = Button(res);
    button4.setPosition(300, 150);
    return true;
}

void LevelMenu::handleEvent(SDL_Event& e, GameState& gs, Resources& res) {
    if (button1.handleEvent(&e, res)) {
        // change state to level 1
    }
    else if (button2.handleEvent(&e, res)) {
        // change state to level 2
    }
}

void LevelMenu::update() {

}

void LevelMenu::render(SDLState* state) {
    SDL_RenderTexture(state->renderer, mBgTexture, nullptr, nullptr);
    button1.render(state);
    button2.render(state);
    button3.render(state);
    button4.render(state);
}

Level::Level(Resources& res):
    map{res.map.get()}
{
}

bool changeState(GameStatus* newState, GameStatus*& currentState, Resources& res) {
    if (newState != nullptr && newState != currentState) {
        currentState = newState;
        currentState->enter(res);
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

bool Button::handleEvent(SDL_Event* e, Resources& res){
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
                    return true; // return true if the mouse is pressed
                    break;

                case SDL_EVENT_MOUSE_BUTTON_UP:
                    mCurrentTexture = res.texButtHov;
                    break;
            }
        }
    }
    return false;
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