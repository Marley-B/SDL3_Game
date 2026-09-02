// manages the difrent game states the game can change through
#include "game_status.h"
#include "level_state.h" 
#include "ui.h"

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
    SDL_Color color = { 0, 0, 255, 255 };
    mBgTexture = res.texLevelMenuScreen;
    mButtons.clear(); // Clear existing buttons
    auto button1 = std::make_unique<Button>(res);
    button1->setPosition(180, 120);
    button1->setText(state, "1", color);
    mButtons.push_back(std::move(button1));
    auto button2 = std::make_unique<Button>(res);
    button2->setPosition(360, 120);
    button2->setText(state, "2", color);
    mButtons.push_back(std::move(button2));
    auto button3 = std::make_unique<Button>(res);
    button3->setPosition(180, 200);
    button3->setText(state, "3", color);
    mButtons.push_back(std::move(button3));
    auto button4 = std::make_unique<Button>(res);
    button4->setPosition(360, 200);
    button4->setText(state, "4", color);
    mButtons.push_back(std::move(button4));
    return true;
}

void LevelMenu::handleEvent(SDL_Event& e, GameState& gs, Resources& res, SDLState& state, GameObject& obj) {
    // Loop trough all the buttons
    for (size_t i = 0; i < mButtons.size(); i++) {
        if (mButtons[i]->handleEvent(&e, res, state)) {
            gs.currentLevel = Level::get(i + 1);
            changeState(Level::get(i + 1), gs.currentStateGame, res, state, gs);
            break;
        }
    }
}

void LevelMenu::update(const SDLState& state, GameState& gs, Resources& res, GameObject& obj, float deltaTime) {}

void LevelMenu::render(SDLState& state, GameState& gs, Resources& res, GameObject& obj, float deltaTime) {
    SDL_RenderTexture(state.renderer, mBgTexture, nullptr, nullptr);
    int i = 0;
    for (auto& button : mButtons) {
        button->render(state, gs);
        if (Level::sLevels[i]->totalButterflies == Level::sLevels[i]->highScore) { // if high score has been achived
            button->renderStar(state);
        }
        i++;
    }
}

DeathState* DeathState::get() {
    return &sDeathState;
}

bool DeathState::enter(SDLState& state, GameState& gs, Resources& res) {
    SDL_Color color = { 0, 0, 255, 255 };
    mBgTexture = res.texGameOverScreen;
    mButtons.clear();
    auto button1 = std::make_unique<Button>(res);
    button1->setPosition(40, 150);
    button1->setText(state, "Retry", color);
    mButtons.push_back(std::move(button1));
    auto button2 = std::make_unique<Button>(res);
    button2->setPosition(40, 220);
    button2->setText(state, "Menu", color);
    mButtons.push_back(std::move(button2));
    MIX_PlayAudio(res.gMixer, res.audioLose);
    return true;
}

void DeathState::handleEvent(SDL_Event& e, GameState& gs, Resources& res, SDLState& state, GameObject& obj) {
    if (mButtons[0]->handleEvent(&e, res, state)) {
        changeState(gs.currentLevel, gs.currentStateGame, res, state, gs);
    }
    else if (mButtons[1]->handleEvent(&e, res, state)) {
        changeState(LevelMenu::get(), gs.currentStateGame, res, state, gs);
    }
}

void DeathState::update(const SDLState& state, GameState& gs, Resources& res, GameObject& obj, float deltaTime) {}

void DeathState::render(SDLState& state, GameState& gs, Resources& res, GameObject& obj, float deltaTime) {
    SDL_RenderTexture(state.renderer, mBgTexture, nullptr, nullptr);
    for (auto& button : mButtons) {
        button->render(state, gs);
    }
}

WinState* WinState::get() {
    return &sWinState;
}

bool WinState::enter(SDLState& state, GameState& gs, Resources& res) {
    coUi = new CoinUi();
    coUi->setPosition(455, 180);
    SDL_Color color = { 0, 0, 255, 255 };
    mBgTexture = res.texWinScreen;
    mButtons.clear();
    auto button = std::make_unique<Button>(res);
    button->setPosition(450, 250);
    button->setText(state, "Menu", color);
    mButtons.push_back(std::move(button));
    MIX_PlayAudio(res.gMixer, res.audioWin);
    return true;
}

void WinState::handleEvent(SDL_Event& e, GameState& gs, Resources& res, SDLState& state, GameObject& obj) {
    if (mButtons[0]->handleEvent(&e, res, state)) {
        changeState(LevelMenu::get(), gs.currentStateGame, res, state, gs);
    }
}

void WinState::update(const SDLState& state, GameState& gs, Resources& res, GameObject& obj, float deltaTime) {}

void WinState::render(SDLState& state, GameState& gs, Resources& res, GameObject& obj, float deltaTime) {
    SDL_RenderTexture(state.renderer, mBgTexture, nullptr, nullptr);
    mButtons[0]->render(state, gs);
    coUi->renderWithCount(state, gs.coinsCollected);
}
bool changeState(GameStatus* newState, GameStatus*& currentState, Resources& res, SDLState& state, GameState& gs) {
    if (newState != nullptr && newState != currentState) {
        currentState = newState;
        currentState->enter(state, gs, res);
        return true;
    }
    return false;
}