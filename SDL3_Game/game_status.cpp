// manages the difrent game states the game can change through
#include "game_status.h"
#include "level_state.h" 

IntroMenu IntroMenu::sIntroMenu;
LevelMenu LevelMenu::sLevelMenu;
DeathState DeathState::sDeathState;
WinState WinState::sWinState;

IntroMenu* IntroMenu::get(){
    return &sIntroMenu;
}

bool IntroMenu::enter(SDLState& state, GameState& gs, Resources& res) {
    mBgTexture = res.texIntroScreen;
    return true;
}

void IntroMenu::handleEvent(SDL_Event& e, GameState& gs, Resources& res, SDLState& state, GameObject& obj) {
    if ((e.type == SDL_EVENT_KEY_DOWN) && ((e.key.key == SDLK_KP_ENTER) || (e.key.key == SDLK_RETURN))) {
        changeState(LevelMenu::get(), gs.currentStateGame , res, state, gs);
    }
}

void IntroMenu::update(const SDLState& state, GameState& gs, Resources& res, GameObject& obj, float deltaTime) {}

void IntroMenu::render(SDLState& state, GameState& gs, Resources& res, GameObject& obj, float deltaTime) {
    SDL_RenderTexture(state.renderer, mBgTexture, nullptr, nullptr);
}

LevelMenu* LevelMenu::get() {
    return &sLevelMenu;
}

bool LevelMenu::enter(SDLState& state, GameState& gs, Resources& res) {
    mBgTexture = res.texLevelMenuScreen;
    button1 = new Button(res);
    button1->setPosition(20, 150);
    button2 = new Button(res);
    button2->setPosition(160, 150);
    button3 = new Button(res);
    button3->setPosition(310, 150);
    button4 = new Button(res);
    button4->setPosition(490, 150);
    return true;
}

void LevelMenu::handleEvent(SDL_Event& e, GameState& gs, Resources& res, SDLState& state, GameObject& obj) {
    if (button1->handleEvent(&e, res, state)) {
        gs.currentLevel = Level::get(1);
        changeState(Level::get(1), gs.currentStateGame, res, state, gs);
    }
    else if (button2->handleEvent(&e, res, state)) {
        gs.currentLevel = Level::get(2);
        changeState(Level::get(2), gs.currentStateGame, res, state, gs);
    }
    else if (button3->handleEvent(&e, res, state)) {
        gs.currentLevel = Level::get(3);
        changeState(Level::get(3), gs.currentStateGame, res, state, gs);
    }
    else if (button4->handleEvent(&e, res, state)) {
        gs.currentLevel = Level::get(4);
        changeState(Level::get(4), gs.currentStateGame, res, state, gs);
    }
}

void LevelMenu::update(const SDLState& state, GameState& gs, Resources& res, GameObject& obj, float deltaTime) {}

void LevelMenu::render(SDLState& state, GameState& gs, Resources& res, GameObject& obj, float deltaTime) {
    SDL_RenderTexture(state.renderer, mBgTexture, nullptr, nullptr);
    button1->render(state, gs);
    button2->render(state, gs);
    button3->render(state, gs);
    button4->render(state, gs);
}

DeathState* DeathState::get() {
    return &sDeathState;
}

bool DeathState::enter(SDLState& state, GameState& gs, Resources& res) {
    mBgTexture = res.texGameOverScreen;
    button1 = new Button(res);
    button1->setPosition(40, 120);
    button2 = new Button(res);
    button2->setPosition(40, 190);
    return true;
}

void DeathState::handleEvent(SDL_Event& e, GameState& gs, Resources& res, SDLState& state, GameObject& obj) {
    if (button1->handleEvent(&e, res, state)) {
        changeState(gs.currentLevel, gs.currentStateGame, res, state, gs);
    }
    else if (button2->handleEvent(&e, res, state)) {
        changeState(LevelMenu::get(), gs.currentStateGame, res, state, gs);
    }
}

void DeathState::update(const SDLState& state, GameState& gs, Resources& res, GameObject& obj, float deltaTime) {}

void DeathState::render(SDLState& state, GameState& gs, Resources& res, GameObject& obj, float deltaTime) {
    SDL_RenderTexture(state.renderer, mBgTexture, nullptr, nullptr);
    button1->render(state, gs);
    button2->render(state, gs);
}

WinState* WinState::get() {
    return &sWinState;
}

bool WinState::enter(SDLState& state, GameState& gs, Resources& res) {
    mBgTexture = res.texWinScreen;
    button = new Button(res);
    button->setPosition(400, 150);
    return true;
}

void WinState::handleEvent(SDL_Event& e, GameState& gs, Resources& res, SDLState& state, GameObject& obj) {
    if (button->handleEvent(&e, res, state)) {
        changeState(LevelMenu::get(), gs.currentStateGame, res, state, gs);
    }
}

void WinState::update(const SDLState& state, GameState& gs, Resources& res, GameObject& obj, float deltaTime) {}

void WinState::render(SDLState& state, GameState& gs, Resources& res, GameObject& obj, float deltaTime) {
    SDL_RenderTexture(state.renderer, mBgTexture, nullptr, nullptr);
    button->render(state, gs);
}
bool changeState(GameStatus* newState, GameStatus*& currentState, Resources& res, SDLState& state, GameState& gs) {
    if (newState != nullptr && newState != currentState) {
        currentState = newState;
        currentState->enter(state, gs, res);
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

bool Button::handleEvent(SDL_Event* e, Resources& res, SDLState& state){
    //If mouse event happened
    if (e->type == SDL_EVENT_MOUSE_MOTION || e->type == SDL_EVENT_MOUSE_BUTTON_DOWN || e->type == SDL_EVENT_MOUSE_BUTTON_UP)
    {
        //Get mouse position
        float x = -1.f, y = -1.f;
        SDL_GetMouseState(&x, &y);

        float logicalMouseX = (x / (float)state.width) * (float)state.logW;
        float logicalMouseY = (y / (float)state.height) * (float)state.logH;

        //Check if mouse is in button
        bool inside = true;

        if (logicalMouseX < mPosition.x){ // left of the button
            inside = false;
        }
        else if (logicalMouseX > mPosition.x + kButtonWidth){ // right of the button
            inside = false;
        }
        else if (logicalMouseY < mPosition.y){ // above the button
            inside = false;
        }
        else if (logicalMouseY > mPosition.y + kButtonHeight ){ //  below the button
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

void Button::render(SDLState& state, GameState &gs) {
    SDL_FRect dst{ // Where on screen to render the sprite
        .x = mPosition.x,
        .y = mPosition.y,
        .w = kButtonWidth,
        .h = kButtonHeight
    };
    SDL_RenderTexture(state.renderer, mCurrentTexture, nullptr, &dst);
}